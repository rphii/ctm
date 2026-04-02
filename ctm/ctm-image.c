#include "ctm.h"
#include "ctm-image.h"
#include "ctm-loader-image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

VEC_IMPLEMENT(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, BASE, ctm_image_free);
VEC_IMPLEMENT(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, ERR);
VEC_IMPLEMENT(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, SORT, ctm_image_cmp);

void ctm_image_free(Ctm_Image *image) {

    if(!image) return;
    if(image->data) {
        stbi_image_free(image->data);
    }
    so_free(&image->filename);
    memset(image, 0, sizeof(*image));

}

int ctm_image_cmp(Ctm_Image *a, Ctm_Image *b) {
    return so_cmp_s(a->filename, b->filename);
}

bool ctm_image_is_valid(Ctm_Image *img) {
    bool is_valid = false;
    if(!pthread_mutex_trylock(&img->mtx)) {
        is_valid = img->loaded && img->tui_image;
        pthread_mutex_unlock(&img->mtx);
    }
    return is_valid;
}

void v_ctm_image_init_from_paths(V_Ctm_Image *v, VSo paths) {
    size_t len = array_len(paths);
    v_ctm_image_resize(v, len);
    for(size_t i = 0; i < len; ++i) {
        So path = array_at(paths, i);
        //printff("Load image %.*s",SO_F(path));
        Ctm_Image *image = v_ctm_image_get_at(v, i);
        image->filename = path;
        image->unique_id = i + 1;
    }
}

