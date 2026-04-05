#include "ctm.h"
#include "ctm-row.h"

void ctm_row_free(Ctm_Row **row) {
    if(row) {
        array_free((*row)->images);
    }
    free(*row);
}


void ctm_row_image_update(Tui_Rect rc_box, Ctm_Config *config, Ctm_Row *row, size_t y0, size_t x0) {
    Tui_Rect rc = {0};
    rc.anc = (Tui_Point){ .x = x0, .y = y0 };
    rc.dim = config->dim_cell;
    Ctm_Image **itE = array_itE(row->images);
    for(Ctm_Image **it = row->images; it < itE; ++it) {
        Ctm_Image *image = *it;
        if(!image) continue;
#if 1
#endif
        //if(rc.anc.x >= x0 && rc.anc.y >= y0) {
            if(!image->render.is_floating) {

                Tui_Rect rc2 = rc;
                if(image->render.is_selected) {
                    rc2.anc.x -= (config->dim_cell_grab.x - config->dim_cell.x) / 2;
                    rc2.anc.y -= (config->dim_cell_grab.y - config->dim_cell.y) / 2;
                    rc2.dim.x = config->dim_cell_grab.x;
                    rc2.dim.y = config->dim_cell_grab.y;
                }

                image->render.rc_image = rc2;
            }
            image->render.rc_cell = rc;

#if 0
        } else {
            image->render.rc_image = (Tui_Rect){0};
        }
#endif
        rc.anc.x += config->dim_cell.x;
        if(rc.anc.x + rc.dim.x > rc_box.dim.x) {
            rc.anc.x = x0;
            rc.anc.y += config->dim_cell.y;
        }
    }
}

size_t ctm_row_get_cols(Ctm_Config *config, Ctm_Row *row, ssize_t width) {

    size_t w_single = config->dim_cell.x;
    size_t w_need = array_len(row->images) * w_single;
    size_t w_have = width ? width : row->render.rc_bg.dim.x;

    if(w_need > w_have) {
        return w_have / w_single;
    } else {
        //exit(1);
        return w_need / w_single;
    }
}

size_t ctm_row_get_rows(Ctm_Config *config, Ctm_Row *row, ssize_t width) {

    size_t n_have = array_len(row->images);
    size_t n_cols = ctm_row_get_cols(config, row, width);
    if(!n_cols) return 1;
    size_t n_rows = (n_have + n_cols - 1) / n_cols;
    return n_rows;

}

void ctm_row_update(Tui_Rect rc_box, Ctm_Config *config, Ctm_Row *row, size_t y0) {

    Tui_Rect rc = {0};

    /* figure out content rc */
    rc.anc.x = config->w_title;
    rc.anc.y = y0;
    size_t n_w = ctm_row_get_cols(config, row, rc_box.dim.x - config->w_title);
    size_t n_h = ctm_row_get_rows(config, row, rc_box.dim.x - config->w_title);
    rc.dim.y = n_h * config->dim_cell.y;
    rc.dim.x = n_w * config->dim_cell.x;
    row->render.rc_images = rc;

    /* figure out backround rcs */
    row->render.rc_bg = row->render.rc_images;
    row->render.rc_bg.dim.x = rc_box.dim.x - config->w_title;

    row->render.rc_ul = row->render.rc_bg;
    row->render.rc_ul.anc.y += row->render.rc_ul.dim.y - 1;
    row->render.rc_ul.dim.y = 1;

    /* figure out title rc */
    rc.anc.x = 0;
    rc.anc.y = y0;
    rc.dim.y = row->render.rc_images.dim.y;
    rc.dim.x = config->w_title;
    row->render.rc_name = rc;

    /* full width */
    row->render.rc_row.anc = row->render.rc_name.anc;
    row->render.rc_row.dim.x = rc_box.dim.x;
    row->render.rc_row.dim.y = row->render.rc_name.dim.y;

    ctm_row_image_update(rc_box, config, row, y0, row->render.rc_images.anc.x);
}

size_t ctm_row_image_index(Ctm_Row *row, Ctm_Image *image) {
    size_t len = array_len(row->images);
    for(size_t i = 0; i < len; ++i) {
        Ctm_Image *im = array_at(row->images, i);
        if(im == image) return i;
    }
    return len;
}

size_t ctm_row_image_index_from_pos(Ctm_Config *config, Ctm_Row *row, Tui_Point pos) {
    size_t len = array_len(row->images);
    size_t result = len;
    Tui_Point pt = tui_rect_project_point(row->render.rc_images, pos);
    if(!pt.x) return len;
    size_t w_cell = config->dim_cell.x;
    size_t h_cell = config->dim_cell.y;
    size_t j = pt.y / h_cell;
    size_t n = ctm_row_get_cols(config, row, row->render.rc_bg.dim.x);
    size_t i = pt.x / w_cell + j * n;
    if(i < len) result = i;
    return result;
}


void ctm_row_pop_image(Ctm *tm, Ctm_Image *image, bool do_delete) {
    if(!image) return;
    /* pop from owner */
    Ctm_Row *row_owner = image->row_owner;
    ASSERT_ARG(row_owner);
    size_t row_owner_len = array_len(row_owner->images);
    ASSERT_ARG(row_owner_len);
    size_t row_owner_index = ctm_row_image_index(row_owner, image);
    if(row_owner_index >= row_owner_len) return;
    size_t row_owner_move = row_owner_len - row_owner_index - 1;

    memmove(row_owner->images + row_owner_index, row_owner->images + row_owner_index + 1, sizeof(*row_owner->images) * row_owner_move);
    array_resize(row_owner->images, row_owner_len - 1);

    if(do_delete && ctm_image_is_valid(image)) {
        tui_image_clear_id_image(tm->tui_core, image->tui_image->id);
    }

}

void ctm_row_image_set(Ctm *tm, Ctm_Row *row, Ctm_Image *image, size_t i) {

    /* insert into certain position */
    if(i + 1 < array_len(row->images)) {

        size_t i0 = ctm_row_image_index(row, image);
        size_t len = array_len(row->images);

        if(row != image->row_owner) {
            ++len;
            ctm_row_pop_image(tm, image, false);
            array_resize(row->images, len);
        }

        if(i0 < i) {
            size_t move = i - i0;
            if(move) {
                memmove(row->images + i0, row->images + i0 + 1, sizeof(*row->images) * move);
            }
        } else {
            size_t move = i0 - i;
            if(move) {
                memmove(row->images + i + 1, row->images + i, sizeof(*row->images) * move);
            }
        }

        row->images[i] = image;
    } else {
        ctm_row_pop_image(tm, image, false);
        array_push(row->images, image);
    }

    /* set new owner */
    image->row_owner = row;

    // TODO: does not get dirty...? ... image->render.is_clean = false;
}

