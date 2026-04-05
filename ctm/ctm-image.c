#include "ctm.h"
#include "ctm-image.h"
#include "ctm-loader-image.h"
#include "ctm-row.h"
#include "ctm-grid.h"

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
    if(!img) return false;
    bool is_valid = false;
    if(!pthread_mutex_trylock(&img->mtx)) {
        is_valid = img->loaded && img->tui_image;
        pthread_mutex_unlock(&img->mtx);
    }
    return is_valid;
}

bool ctm_image_is_loaded(Ctm_Image *img) {
    if(!img) return false;
    bool is_loaded = false;
    if(!pthread_mutex_trylock(&img->mtx)) {
        is_loaded = img->loaded;
        pthread_mutex_unlock(&img->mtx);
    }
    return is_loaded;
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

bool ctm_image_unselect(Ctm_Image *image) {
    if(!image) return 0;
    bool changed = !image->render.is_clean
        || image->render.is_selected;

    image->render.is_clean &= !image->render.is_selected; /* mark as not clean if was selected */
    image->render.is_selected = false;
    if(ctm_image_is_valid(image)) {
        changed |= (image->tui_image->z != CTM_IMG_Z_INDEX);
        image->tui_image->z = CTM_IMG_Z_INDEX;
    }
    return changed;
}

bool ctm_image_unfloat(Ctm_Image *image) {
    if(!image) return 0;
    bool changed = !image->render.is_clean
        || image->render.is_floating;

    image->render.is_clean &= !image->render.is_floating; /* mark as not clean if was floating */
    image->render.is_floating = false;
    if(ctm_image_is_valid(image)) {
        changed |= (image->tui_image->z != CTM_IMG_Z_INDEX);
        image->tui_image->z = CTM_IMG_Z_INDEX;
    }
    return changed;
}

bool ctm_image_unboth(Ctm_Image *image) {
    if(!image) return 0;
    bool changed = !image->render.is_clean
        || image->render.is_floating
        || image->render.is_selected;

    if(!image) return false;
    image->render.is_clean &= !image->render.is_floating; /* mark as not clean if was floating */
    image->render.is_floating = false;
    image->render.is_selected = false;
    if(ctm_image_is_valid(image)) {
        changed |= (image->tui_image->z != CTM_IMG_Z_INDEX);
        image->tui_image->z = CTM_IMG_Z_INDEX;
    }
    return changed;
}

size_t ctm_image_get_grid_y(Ctm_Config *config, Ctm_Grid *grid, Ctm_Image *image) {
    size_t len = array_len(grid->rows);
    size_t ycum = 0; // ctm_grid_row_index(grid, image->row_owner);
    for(size_t i = 0; i < len; ++i) {
        Ctm_Row *row = array_at(grid->rows, i);
        if(row == image->row_owner) break;
        size_t rows = ctm_row_get_rows(config, row, row->render.rc_bg.dim.x);
        if(!rows) ++rows;
        ycum += rows;
    }
    ssize_t cols = ctm_row_get_cols(config, image->row_owner, image->row_owner->render.rc_bg.dim.x);
    ssize_t index = ctm_row_image_index(image->row_owner, image);
    if(!index) return ycum;
    ycum += index / cols;
    return ycum;
}

Ctm_Image *ctm_image_find_first_best(Ctm_Config *config, Ctm_Grid *grid, Ctm_Row *row0, Ctm_Image *fallback, int direction) {
    if(!direction) return fallback;
    /* find first */
    Ctm_Row **itE = direction >= 0 ? array_itE(grid->rows) : grid->rows;
    Ctm_Row **it0 = direction <  0 ? array_itL(grid->rows) : grid->rows;
    //if(row0) it0 = row0 + direction;
    Ctm_Image *result = 0;
    bool found_first = (row0 == 0);
    for(Ctm_Row **it = it0; direction >= 0 ? it < itE : it + 1 > itE; it += direction) {
        Ctm_Row *row = *it;
        if(!row) continue;
        if(row == row0) {
            found_first = true;
            continue;
        }
        if(!found_first) continue;
        if(array_len(row->images)) {
            result = *row->images;
            break;
        }
    }
    return result ? result : fallback;
}


