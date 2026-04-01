#include "ctm-loader-image.h"
#include <stb/stb_image.h>

#include "ctm-image.h"
#include "ctm.h"

typedef struct Ctm_Queue_Do {
    Ctm *ctm;
    Ctm_Image *image;
} Ctm_Queue_Do;

void ctm_loader_image_init(Ctm_Loader_Image *loader, struct Ctm *ctm, size_t n_jobs) {
    loader->ctm = ctm;
    pw_init(&loader->pw, n_jobs ? n_jobs : 1);
    pw_dispatch(&loader->pw);
}


void *ctm_loader_image_image(Pw *pw, bool *cancel, void *user) {
    Ctm_Queue_Do *qd = user;
    Ctm_Image *load = qd->image;

    bool loaded = false;
    FILE *fp = so_file_fp(load->filename, "r");
    //printff("Opened file %.*s!", SO_F(load->filename));

    pthread_rwlock_wrlock(&load->rwlock);
    if(fp) {
        load->data = stbi_load_from_file(fp, &load->width, &load->height, &load->channels, 0);
        if(load->data) {
            loaded = true;
        }
        load->loaded = true;
        fclose(fp);
    }
    pthread_rwlock_unlock(&load->rwlock);

    if(loaded) {
        pthread_mutex_lock(&qd->ctm->events.mtx);
        ++qd->ctm->events.thumb_loaded;
        pthread_mutex_unlock(&qd->ctm->events.mtx);
        tui_sync_main_update(&qd->ctm->tui_sync.main);
    }

    free(qd);
    return 0;
}

void ctm_loader_image_add(Ctm_Loader_Image *loader, struct Ctm_Image *image) {
    Ctm_Queue_Do *qd;
    NEW(Ctm_Queue_Do, qd);
    qd->ctm = loader->ctm;
    qd->image = image;
    pw_queue(&loader->pw, ctm_loader_image_image, qd);
}

