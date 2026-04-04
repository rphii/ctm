#include "ctm-grid.h"
#include "ctm.h"

void ctm_grid_update(Tui_Rect rc_grid, Ctm_Config *config, Ctm_Grid *grid) {
    size_t y0 = rc_grid.anc.y;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        ctm_row_update(rc_grid, config, *it, y0);
        y0 += (*it)->render.rc_name.dim.y;
    }
}

void ctm_grid_all_dirty(Ctm_Grid *grid) {
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(!image) continue;
            image->render.is_clean = false;
            (void) ctm_image_unfloat(image);
        }
    }
}

bool ctm_grid_update_dirty(Ctm_Grid *grid, bool unfloat_all, bool unselect_all) {
    bool render = false;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(!image) continue;
            if(tui_rect_cmp(image->render.rc_image, image->render.rc_image_prev)) {
                image->render.is_clean = false;
                render |= true;
            }
            if(unfloat_all && unselect_all) {
                render |= ctm_image_unboth(image);
            } else if(unfloat_all) {
                render |= ctm_image_unfloat(image);
            } else if(unselect_all) {
                render |= ctm_image_unselect(image);
            }
        }
    }
    return render;
}

size_t ctm_grid_row_index(Ctm_Grid *grid, Ctm_Row *row) {
    size_t len = array_len(grid->rows);
    for(size_t i = 0; i < len; ++i) {
        Ctm_Row *im = array_at(grid->rows, i);
        if(im == row) return i;
    }
    return len;
}

Ctm_Image *ctm_grid_image_from_pos(Ctm_Grid *grid, Tui_Point pos) {
    Ctm_Row **itE = array_itE(grid->rows);
    Ctm_Image *result = 0;
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(tui_rect_encloses_point(image->render.rc_image, pos)) {
                result = image;
                goto break2;
            }
        }
        continue; break2: break;
    }
    return result;
}


void ctm_grid_change_xy(Ctm_Config *config, Ctm_Grid *grid, Ctm_Image *image, ssize_t change_x, ssize_t change_y, ssize_t *col_i, Ctm_Row **row_change, bool skip_empty) {
    ssize_t cols = ctm_row_get_cols(config, image->row_owner, image->row_owner->render.rc_bg.dim.x);
    ssize_t rows = ctm_row_get_rows(config, image->row_owner, image->row_owner->render.rc_bg.dim.x);
    ssize_t cols_real = array_len(image->row_owner->images);
    ssize_t x_at = ctm_row_image_index(image->row_owner, image);
    ssize_t y_at = ctm_grid_row_index(grid, image->row_owner);
    ssize_t col_at = x_at % cols;
    ssize_t row_at = x_at / cols; /* within its grid_row */
    ssize_t cols_this = cols;
    if(row_at + 1 >= rows && cols_real > cols && cols_real % cols) {
        cols_this = cols_real % cols;
    }

    ssize_t x = 0, y = 0, y_move = 0;

    ssize_t go_x = change_x;
    ssize_t go_y = change_y;

    if(go_x) {
        ssize_t col_new = col_at + go_x;
        if(col_new < 0) {
            col_new = cols_this - 1;
        }
        if(col_new >= cols_this) {
            col_new = 0;
        }
        //if(col_new 
        x = col_new;
    } else {
        x = col_at;
    }

    if(go_y) {
        ssize_t row_new = row_at + go_y;
        if(row_new < 0) {
            row_new = 0;
            y_move -= 1; /* TODO .. */
        }
        if(row_new >= rows) {
            row_new = rows - 1;
            y_move += 1; /* TODO .. */
        }
        y = row_new;
    } else {
        y = row_at;
    }

    Ctm_Row *row_new = image->row_owner;
    Ctm_Row *row_new_changed = 0;

    if(y_move) {
        ssize_t y_new = y_at + y_move;
        if(skip_empty) {
            Ctm_Image *image_row = ctm_image_find_first_best(config, grid, image->row_owner, 0, y_move);
            if(image_row) {
                row_new_changed = image_row->row_owner;
            }
        } else {
            if(y_new >= 0 && y_new < array_len(grid->rows)) {
                row_new_changed = array_at(grid->rows, y_new);
            }
        }
    }

    if(row_new_changed) {
        row_new = row_new_changed;
        cols_real = array_len(row_new->images);
        if(y_move < 0) {

            size_t n_rows = ctm_row_get_rows(config, row_new, row_new->render.rc_bg.dim.x);
            y = n_rows ? n_rows - 1 : 0;
            y = 0;

            size_t n_cols = ctm_row_get_cols(config, row_new, row_new->render.rc_bg.dim.x);
            if(n_cols) {
                size_t n_cols_last = array_len(row_new->images) % n_cols - 1;

                size_t result = x > n_cols_last ? n_cols_last : x;
                result += n_cols * (n_rows - 1);
                if(result >= cols_real) result = cols_real - 1;

                *col_i = result;
                *row_change = row_new;
                return;
            }

        } else if(y_move > 0) {
            y = 0;
        }
    }

    ssize_t i_col = x + y * cols;
    if(i_col >= cols_real) i_col = cols_real - 1;

    *col_i = i_col;
    *row_change = row_new;
}


