#ifndef CTM_LOADER_IMAGE_H

#include <rlpw.h>

struct Ctm;
struct Ctm_Image;

typedef struct Ctm_Loader_Image {

    struct Ctm *ctm;
    Pw pw;
    pthread_mutex_t images_mtx;

    struct {
        size_t thumb_w;
        size_t thumb_h;
    } config;

} Ctm_Loader_Image;

void ctm_loader_image_init(Ctm_Loader_Image *loader, struct Ctm *ctm, size_t n_jobs);
void ctm_loader_image_add(Ctm_Loader_Image *loader, struct Ctm_Image *image);

#define CTM_LOADER_IMAGE_H
#endif /* CTM_LOADER_IMAGE_H */

