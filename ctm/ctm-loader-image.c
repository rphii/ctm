#include "ctm-loader-image.h"
#include <stb/stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize2.h>

#include "ctm-image.h"
#include "ctm.h"

typedef struct Ctm_Queue_Do {
    Ctm *ctm;
    Ctm_Image *image;
} Ctm_Queue_Do;

void ctm_loader_image_init(Ctm_Loader_Image *loader, struct Ctm *ctm, size_t n_jobs) {
    loader->ctm = ctm;
    loader->config.thumb_h = 128;
    loader->config.thumb_w = 128;
    pw_init(&loader->pw, n_jobs ? n_jobs : 1);
    pw_dispatch(&loader->pw);

    for(size_t i = 0; i < v_ctm_image_length(ctm->v_images); ++i) {
        Ctm_Image *image = v_ctm_image_get_at(&ctm->v_images, i);
        ctm_loader_image_add(loader, image);
    }

}


void *ctm_loader_image_image(Pw *pw, bool *cancel, void *user) {
    Ctm_Queue_Do *qd = user;
    Ctm_Image *load = qd->image;

    bool loaded = false;
    FILE *fp = so_file_fp(load->filename, "r");
    //printff("Opened file %.*s!", SO_F(load->filename));

    pthread_mutex_lock(&load->mtx);
    if(fp) {
        int w, h, ch;
        uint8_t *data = stbi_load_from_file(fp, &w, &h, &ch, 0);
        if(!data) goto defer;

        load->channels = ch;
        load->width = qd->ctm->loader_image.config.thumb_w;
        load->height = qd->ctm->loader_image.config.thumb_h;
        load->data = malloc(load->width * load->height * load->channels);
        if(!load->data) goto defer;

        stbir_resize_uint8_linear(data, w, h, 0, load->data, load->width, load->height, 0, load->channels);
        load->tui_image = tui_image_new(qd->ctm->tui_core, load->unique_id, load->data, (Tui_Point){ .x = load->width, .y = load->height }, load->channels);
        load->loaded = true;
        fclose(fp);
    }
defer:
    loaded = load->loaded;
    pthread_mutex_unlock(&load->mtx);

    if(loaded) {
        pthread_mutex_lock(&qd->ctm->events.mtx);
        ++qd->ctm->events.thumb_loaded;
        pthread_mutex_unlock(&qd->ctm->events.mtx);
        /* TODO: use sync_main_update -> but it does not work, so we just use sync_main_both lol */
        //tui_sync_main_update(&qd->ctm->tui_sync.main);
        tui_sync_main_both(&qd->ctm->tui_sync.main);
    }

    free(qd);
    return 0;
}

void *ctm_loader_when_all_done(Pw *pw, bool *cancel, void *user) {
    Ctm_Loader_Image *loader = user;
    
    /* TODO: use sync_main_update -> but it does not work, so we just use sync_main_both lol */
    tui_sync_main_both(&loader->ctm->tui_sync.main);
    return 0;
}

void ctm_loader_image_add(Ctm_Loader_Image *loader, struct Ctm_Image *image) {
    Ctm_Queue_Do *qd;
    NEW(Ctm_Queue_Do, qd);
    qd->ctm = loader->ctm;
    qd->image = image;
    pw_queue(&loader->pw, ctm_loader_image_image, qd);
    pw_when_done(&loader->pw, ctm_loader_when_all_done, loader);
}

