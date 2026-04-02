#ifndef CTM_IMAGE_H

#include <rlso.h>
#include <rlc.h>
#include <rltui.h>

struct Ctm_Loader_Image;

typedef struct Ctm_Image {
    So filename;
    uint8_t *data;
    int width;
    int height;
    int channels;
    bool loaded;
    int unique_id;
    pthread_rwlock_t rwlock; /* mutex for data */
    //Tui_Image *tui_image;
} Ctm_Image;

void ctm_image_free(Ctm_Image *img);
int ctm_image_cmp(Ctm_Image *a, Ctm_Image *b);

VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, BASE);
VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, ERR);
VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, SORT);

void v_ctm_image_init_from_paths(V_Ctm_Image *v, struct Ctm_Loader_Image *loader, VSo paths);

#define CTM_IMAGE_H
#endif /* CTM_IMAGE_H */

