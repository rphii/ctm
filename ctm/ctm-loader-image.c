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
    loader->config.thumb_h = loader->ctm->config.thumb;
    loader->config.thumb_w = loader->ctm->config.thumb;
    pw_init(&loader->pw, n_jobs ? n_jobs : 1);
    pw_dispatch(&loader->pw);

    for(size_t i = 0; i < v_ctm_image_length(ctm->v_images); ++i) {
        Ctm_Image *image = v_ctm_image_get_at(&ctm->v_images, i);
        ctm_loader_image_add(loader, image);
    }

}

void ctm_loader_set_fallback(Ctm_Image *image, uint8_t r, uint8_t g, uint8_t b) {

    image->render.fallback_bg.r = r;
    image->render.fallback_bg.g = g;
    image->render.fallback_bg.b = b;

    uint8_t bright = color_as_brightness((Color){ .r = r, .g = g, .b = b, .a = 0xFF }, COLOR_GAMMA_DEFAULT);
    //printff("bright %u", bright);

    image->render.fallback_fg.r = bright > 50 ? 0 : 0xFF;
    image->render.fallback_fg.g = bright > 50 ? 0 : 0xFF;
    image->render.fallback_fg.b = bright > 50 ? 0 : 0xFF;

    image->render.fallback_bg.type = TUI_COLOR_RGB;
    image->render.fallback_fg.type = TUI_COLOR_RGB;
}

void *ctm_loader_image_image(Pw *pw, bool *cancel, void *user) {
    Ctm_Queue_Do *qd = user;
    Ctm_Image *load = qd->image;

    bool loaded = false;
    FILE *fp = so_file_fp(load->filename, "r");
    uint8_t *data = 0;
    //printff("Opened file %.*s!", SO_F(load->filename));

    pthread_mutex_lock(&load->mtx);
    if(fp) {
        int w, h, ch;
        data = stbi_load_from_file(fp, &w, &h, &ch, 0);
        if(!data) goto defer;

        load->channels = ch;
        load->width = qd->ctm->loader_image.config.thumb_w;
        load->height = qd->ctm->loader_image.config.thumb_h;
        load->data = malloc(load->width * load->height * load->channels);
        if(!load->data) goto defer;

        /* if there is a universe with 2 channel images, I don't want them */
        if(load->channels == 2) {
            free(load->data);
            goto defer;
        }

        stbir_resize_uint8_linear(data, w, h, 0, load->data, load->width, load->height, 0, load->channels);
        load->tui_image = tui_image_new(qd->ctm->tui_core, load->unique_id, load->data, (Tui_Point){ .x = load->width, .y = load->height }, load->channels);
        load->loaded = true;
        fclose(fp);

        /* estimate average color by simply resizing it to 1 pixel */
        uint8_t color[load->channels < 3 ? 3 : load->channels];
        stbir_resize_uint8_linear(load->data, load->width, load->height, 0, color, 1, 1, 0, load->channels);
        if(load->channels < 3) {
            color[1] = color[0];
            color[2] = color[0];
        }
        ctm_loader_set_fallback(load, color[0], color[1], color[2]);

    }
defer:
    loaded = load->loaded;
    pthread_mutex_unlock(&load->mtx);

    free(data);

    if(loaded) {
        pthread_mutex_lock(&qd->ctm->events.mtx);
        ++qd->ctm->events.thumb_loaded;
        pthread_mutex_unlock(&qd->ctm->events.mtx);
        /* TODO: use sync_main_update -> but it does not work, so we just use sync_main_both lol */
        //tui_sync_main_update(&qd->ctm->tui_sync.main);
        tui_sync_main_both(&qd->ctm->tui_sync.main);
    } else {
        ctm_loader_set_fallback(load, rand(), rand(), rand());
    }

    free(qd);
    return 0;
}

void *ctm_loader_when_all_done(Pw *pw, bool *cancel, void *user) {
    Ctm_Loader_Image *loader = user;
    
    /* TODO: use sync_main_update -> but it does not work, so we just use sync_main_both lol */
    //tui_sync_main_update(&loader->ctm->tui_sync.main);
    tui_sync_main_both(&loader->ctm->tui_sync.main);
    pw_when_done_clear(pw);
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

