#include <rltui.h>
#include <rlarg.h>
#include <rlso.h>


// TODO : rltui -> what if rc changes, and I get an input ?? do I mutex lock the update/input ?

#include "ctm.h"
#include "ctm-arg.h"
#include "ctm-image.h"
#include "ctm-loader-image.h"

bool ctm_input(Tui_Input *input, bool *flush, void *user) {
    Ctm *tm = user;
    Ctm_Input tm_input = tm->input;
    bool update = false;
    switch(input->id) {
        case INPUT_TEXT: {
            switch(input->text.val) {
                case 'q': {
                    tui_core_quit(tm->tui_core);
                } break;
                case 'j': {
                    ++tm->input.move_y;
                } break;
                case 'k': {
                    --tm->input.move_y;
                } break;
                case 'l': {
                    ++tm->input.move_x;
                } break;
                case 'h': {
                    --tm->input.move_x;
                } break;
                case ' ': {
                    tm->input.space = true;
                } break;
            }
        } break;
        case INPUT_CODE: {
        } break;
        case INPUT_MOUSE: {
            tm->input.mouse = input->mouse;
        } break;
        default: break;
    }

    update = memcmp(&tm_input, &tm->input, sizeof(tm_input));
    return update;
}

void ctm_row_image_update(Tui_Point dimensions, Ctm_Grid *grid, Ctm_Row *row, size_t y0, size_t x0) {
    Tui_Rect rc = {0};
    rc.anc = (Tui_Point){ .x = x0, .y = y0 };
    rc.dim = grid->dim_cell;
    Ctm_Image **itE = array_itE(row->images);
    for(Ctm_Image **it = row->images; it < itE; ++it) {
        Ctm_Image *image = *it;
        if(!image) continue;
#if 1
#endif
        //if(rc.anc.x >= x0 && rc.anc.y >= y0) {
            if(!image->render.is_floating) {
                image->render.rc_image = rc;
            }
#if 0
        } else {
            image->render.rc_image = (Tui_Rect){0};
        }
#endif
        rc.anc.x += grid->dim_cell.x;
        if(rc.anc.x + rc.dim.x >= dimensions.x) {
            rc.anc.x = x0;
            rc.anc.y += grid->dim_cell.y;
        }
    }
}

size_t ctm_row_get_rows(Ctm_Grid *grid, Ctm_Row *row) {
    size_t w_need = array_len(row->images) * grid->dim_cell.x;
    size_t w = grid->dim_cell.x;
    if(!w_need) return 1;
    size_t n_need = (w_need) / w;
    size_t n_have = (grid->render.rc_grid.dim.x - grid->w_title) / w;
    size_t n_h = (n_need + n_have - 1) / n_have;
    if(!n_h) return 1;
    return n_h;
}

size_t ctm_row_get_cols(Ctm_Grid *grid, Ctm_Row *row) {

    size_t w_single = grid->dim_cell.x;
    size_t w_need = array_len(row->images) * w_single;
    size_t w_have = grid->render.rc_grid.dim.x - grid->w_title;

    if(w_need > w_have) {
        return w_have / w_single;
    } else {
        return w_need / w_single;
    }
}

void ctm_row_update(Tui_Point dimensions, Ctm_Grid *grid, Ctm_Row *row, size_t y0) {
    grid->render.rc_grid.dim = dimensions;

    Tui_Rect rc = {0};
    
    /* figure out content rc */
    rc.anc.x = grid->w_title;
    rc.anc.y = y0;
    size_t n_w = ctm_row_get_cols(grid, row);
    size_t n_h = ctm_row_get_rows(grid, row);
    rc.dim.y = n_h * grid->dim_cell.y;
    rc.dim.x = n_w * grid->dim_cell.x;
    row->render.rc_images = rc;

    /* figure out backround rcs */
    row->render.rc_bg = row->render.rc_images;
    row->render.rc_bg.dim.x = dimensions.x - grid->w_title;

    row->render.rc_ul = row->render.rc_bg;
    row->render.rc_ul.anc.y += row->render.rc_ul.dim.y - 1;
    row->render.rc_ul.dim.y = 1;

    /* figure out title rc */
    rc.anc.x = 0;
    rc.anc.y = y0;
    rc.dim.y = row->render.rc_images.dim.y;
    rc.dim.x = grid->w_title;
    row->render.rc_name = rc;

    /* full width */
    row->render.rc_row.anc = row->render.rc_name.anc;
    row->render.rc_row.dim.x = dimensions.x;
    row->render.rc_row.dim.y = row->render.rc_name.dim.y;

    ctm_row_image_update(dimensions, grid, row, y0, row->render.rc_images.anc.x);
}

void ctm_grid_update(Tui_Point dimension, Ctm_Grid *grid) {
    size_t y0 = 0;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        ctm_row_update(dimension, grid, *it, y0);
        y0 += (*it)->render.rc_name.dim.y;
    }
}

#if 0
void ctm_image_select_first_best(Ctm_Grid *grid, Ctm_Image_Select *select) {
    /* find first */
    size_t y = 0;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        if(!row) continue;
        if(array_len(row->images)) {
            select->select.image = *row->images;
            ++y;
            break;
        }
        y += ctm_row_get_rows(grid, row);
    }
    select->select.id = CTM_IMAGE_SELECT_GRID;
    select->select.grid_index.x = 0;
    select->select.grid_index.y = y ? y - 1 : 0;
}

void ctm_image_select_move_y(Ctm *tm, Ctm_Image_Select *select, ssize_t n) {
    if(!n) return;
    if(select->select.grabbed) {
    } else {
        select->select.grid_index.y += n;
    }
}

void ctm_image_select_move_x(Ctm *tm, Ctm_Image_Select *select, ssize_t n) {
    if(!n) return;
    if(select->select.grabbed) {
    } else {
        select->select.grid_index.x += n;
    }
}
#endif

bool ctm_grid_index_image(Ctm_Grid *grid, Ctm_Row *row, Ctm_Image *image, size_t *index) {
    size_t result = 0;
    return false;
}

void ctm_image_unfloat(Ctm_Image *image) {
    image->render.is_clean &= !image->render.is_floating; /* mark as not clean if was floating */
    image->render.is_floating = false;
    if(ctm_image_is_valid(image)) {
        image->tui_image->z = 0;
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
            ctm_image_unfloat(image);
        }
    }
}

void ctm_grid_update_dirty(Ctm_Grid *grid, bool un_float_all) {
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(!image) continue;
            if(tui_rect_cmp(image->render.rc_image, image->render.rc_image_prev)) {
                image->render.is_clean = false;
            }
            if(un_float_all) {
                ctm_image_unfloat(image);
            }
        }
    }
}

void ctm_image_select_update(Ctm_Grid *grid, Ctm_Image_Select *select) {
    select->render.rc = (Tui_Rect){0};
    if(!select->select.image) {
    } else {
        Ctm_Image *image = select->select.image;
        // select->render.rc = image->render.rc_image;
        // image->render.rc_image.anc.x += 2;
        // image->render.rc_image.anc.y += 1;
        // image->render.rc_image.dim.x -= 4;
        // image->render.rc_image.dim.y -= 2;
    }
    //select->render.rc.anc.y = grid->dim_cell.y * select->select.grid_index.y;
}

size_t ctm_row_image_index(Ctm_Row *row, Ctm_Image *image) {
    size_t len = array_len(row->images);
    for(size_t i = 0; i < len; ++i) {
        Ctm_Image *im = array_at(row->images, i);
        if(im == image) return i;
    }
    return len;
}

size_t ctm_row_image_index_from_pos(Ctm_Grid *grid, Ctm_Row *row, Tui_Point pos) {
    size_t len = array_len(row->images);
    size_t result = len;
    Tui_Point pt = tui_rect_project_point(row->render.rc_images, pos);
    if(!pt.x) return len;
    size_t w_cell = grid->dim_cell.x;
    size_t i = pt.x / w_cell;
    if(i < len) result = i;
    return result;
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
                //if(ctm_image_is_valid(image)) {
                    result = image;
                    goto break2;
                //} else {
                //}
            }
        }
        continue; break2: break;
    }
    return result;
}


void ctm_row_pop_image(Ctm *tm, Ctm_Image *image, bool delete) {
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

    if(delete && ctm_image_is_valid(image)) {
        tui_image_clear_id_image(tm->tui_core, image->tui_image->id);
    }

}

bool ctm_update(void *user) {
    Ctm *tm = user;
    bool render = false;

    if(tm->input.mouse.m.press) {
        Ctm_Grid *grid = &tm->grid;
        Ctm_Image *selected = ctm_grid_image_from_pos(grid, tm->input.mouse.pos);
        ctm_row_pop_image(tm, selected, true);
    }

    if(tm->input.mouse.l.press) {

        Ctm_Grid *grid = &tm->grid;
        Ctm_Image *selected = ctm_grid_image_from_pos(grid, tm->input.mouse.pos);
        if(selected) {
            tm->image_select.select.image = selected;
            // TODO : needs to check if is_valid ++tm->image_select.select.image->tui_image->z;
            tm->image_select.select.float_origin = tm->input.mouse.pos;
            tm->image_select.select.float_anc = selected->render.rc_image.anc;
            tm->image_select.select.image->render.is_floating = true;
            tm->image_select.select.image->render.is_clean = false;
        }

    }

    bool un_float_all = false;
    if(tm->image_select.select.image) {
        Ctm_Image *selected = tm->image_select.select.image;
        if(tm->input.mouse.l.repeat && selected->render.is_floating) {
            Tui_Point p0 = tm->image_select.select.float_anc;
            Tui_Point p1 = tm->image_select.select.float_origin;
            Tui_Point p2 = tm->input.mouse.pos;
            /* new anc = p0 + (p2 - p1) */
            selected->render.rc_image.anc.x = p0.x + (p2.x - p1.x);
            selected->render.rc_image.anc.y = p0.y + (p2.y - p1.y);
        }
        if(tm->input.mouse.l.release) {
            if(selected->render.is_floating) {
                un_float_all = true;
                /* find rect that would be the target for the drop */
                Ctm_Row **itE = array_itE(tm->grid.rows);
                for(Ctm_Row **it = tm->grid.rows; it < itE; ++it) {
                    Ctm_Row *row = *it;
                    //printff("rc %u %u %u %u",row->render.rc_grid.anc.x,row->render.rc_grid.anc.y,row->render.rc_grid.dim.x,row->render.rc_grid.dim.y);
                    if(tui_rect_encloses_point(row->render.rc_row, tm->input.mouse.pos)) {

                        /* insert into certain position */
                        size_t i = ctm_row_image_index_from_pos(&tm->grid, row, tm->input.mouse.pos);
                        if(i + 1 < array_len(row->images)) {

                            size_t i0 = ctm_row_image_index(row, selected);
                            size_t len = array_len(row->images);

                            if(row != selected->row_owner) {
                                ++len;
                                ctm_row_pop_image(tm, selected, false);
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

                            row->images[i] = selected;
                        } else {
                            ctm_row_pop_image(tm, selected, false);
                            array_push(row->images, selected);
                        }

                        /* set new owner */
                        selected->row_owner = row;

                        // TODO: does not get dirty ... selected->render.is_clean = false;
                    }
                }
            }
        }
    }

    ctm_grid_update(tm->dimensions, &tm->grid);

#if 1
    if(!tm->image_select.select.image) {
        //ctm_image_select_first_best(&tm->grid, &tm->image_select);
    }

    //ctm_image_select_move_x(tm, &tm->image_select, tm->input.move_x);
    //ctm_image_select_move_y(tm, &tm->image_select, tm->input.move_y);

    ctm_image_select_update(&tm->grid, &tm->image_select);
#endif

    ctm_grid_update_dirty(&tm->grid, un_float_all);

    //tm->input_mouse_prev = tm->input.mouse;
    tm->input = (Ctm_Input){0};
    return render;
}

void ctm_render(Tui_Buffer *buffer, void *user) {
    Ctm *tm = user;

    So tmp = SO;
    Tui_Color fg_images = { .type = TUI_COLOR_8, .col8 = 2 };
    Tui_Rect rc_images = {
        .anc = (Tui_Point){ .x = 0, .y = 0 },
        .dim = (Tui_Point){ .x = buffer->dimension.x, .y = 1 }
    };

    Ctm_Grid *grid = &tm->grid;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        tui_buffer_draw(buffer, row->render.rc_bg, 0, &row->render.bg_bg, 0, SO);
        tui_buffer_draw(buffer, row->render.rc_ul, &row->render.fg_ul, &row->render.bg_bg, &(Tui_Fx){ .ul = true }, SO);
        tui_buffer_draw(buffer, row->render.rc_name, &row->fg, &row->bg, &row->fx, row->name);
    }

    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;

        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(tm->config.is_graphics_supported && ctm_image_is_valid(image)) {
                /* make sure the image is updated on the TUI side */
                if(!image->render.is_send_error && !image->render.is_send_ok) {
                    image->render.is_send_error = tui_image_update(tm->tui_core, image->tui_image, 0);
                    image->render.is_send_ok = !image->render.is_send_error;
                }
                /* render only if dirty */
                if(!image->render.is_clean) {
                    //printff("place image @ %u,%u",image->render.rc_image.anc.x,image->render.rc_image.anc.y);
                    image->render.rc_image_prev = image->render.rc_image;
                    image->render.is_clean = true;
                    //tui_image_clear_id_place(tm->tui_core, &tmp, image->tui_image->id);
                    image->tui_image->dst = image->render.rc_image;
                    image->tui_image->src.dim = image->tui_image->dimensions;
                    image->tui_image->z = 0;

                    if(tui_rect_contains_rect(row->render.rc_images, image->render.rc_image) || image->render.is_floating) {
                        tui_image_render(tm->tui_core, image->tui_image, image->tui_image->id, 0);
                    } else {
                        tui_image_clear_id_image(tm->tui_core, image->tui_image->id);
                    }
                }
            } else {
                tui_buffer_draw(buffer,image->render.rc_image, &image->render.fallback_fg, &image->render.fallback_bg, 0, so_get_nodir(image->filename));
            }
        }

    }

    tui_buffer_draw(buffer, tm->image_select.render.rc, 0, &tm->image_select.render.bg, 0, SO);

    so_free(&tmp);

}

void ctm_resized(Tui_Point size, Tui_Point pixels, void *user) {
    Ctm *tm = user;
    ctm_grid_all_dirty(&tm->grid);
    tm->dimensions = size;

    //tm->config.is_graphics_supported = true;
}

void ctm_row_free(Ctm_Row **row) {
    if(row) {
        array_free((*row)->images);
    }
    free(*row);
}

void ctm_grid_free(Ctm_Grid *grid) {
    array_free_ext(grid->rows, ctm_row_free);
}

int main(int argc, const char **argv) {

    int err = 0;

    Ctm tm = {0};

    /* set up argument config */
    tm.arg_config = arg_config_new(),
    arg_config_set_program(tm.arg_config, so_l(argv[0]));
    arg_config_set_description(tm.arg_config, so("C Tier Maker (create tierlists in your terminal)"));

    /* set up argument parser */
    tm.arg = arg_new(tm.arg_config);
    ctm_arg(&tm);

    /* parse arguments */
    err = arg_parse(tm.arg, argc, argv, &tm.arg_quit_early);
    if(err || tm.arg_quit_early) goto defer;

    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    /* set up the tui portion of tui */
    tm.tui_defer = true;
    if(tm.tui_defer) {
        tm.tui_core = tui_core_new();
        tm.tui_core_callbacks.input = ctm_input;
        tm.tui_core_callbacks.update = ctm_update;
        tm.tui_core_callbacks.render = ctm_render;
        tm.tui_core_callbacks.resized = ctm_resized;
        tui_enter();
        tui_core_init(tm.tui_core, &tm.tui_core_callbacks, &tm.tui_sync, &tm);
        tm.config.is_graphics_supported = tui_image_is_supported(tm.tui_core);
    }

    /* init all other ctm structs */

    v_ctm_image_init_from_paths(&tm.v_images, tm.image_paths);

    Ctm_Row *row;
    NEW(Ctm_Row, row);
    row->name = so("S");
    row->bg = (Tui_Color){ .r = 0xFF, .g = 0x00, .b = 0x00, .type = TUI_COLOR_RGB };
    row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("A");
    row->bg = (Tui_Color){ .r = 0x80, .g = 0x00, .b = 0x00, .type = TUI_COLOR_RGB };
    //row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("B");
    row->bg = (Tui_Color){ .r = 0x6F, .g = 0x20, .b = 0x00, .type = TUI_COLOR_RGB };
    row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("C");
    row->bg = (Tui_Color){ .r = 0x20, .g = 0x60, .b = 0x00, .type = TUI_COLOR_RGB };
    //row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("D");
    row->bg = (Tui_Color){ .r = 0x00, .g = 0x80, .b = 0x00, .type = TUI_COLOR_RGB };
    row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("E");
    row->bg = (Tui_Color){ .r = 0x00, .g = 0xFF, .b = 0x00, .type = TUI_COLOR_RGB };
    //row->render.bg_bg = (Tui_Color){ .r = 0x4, .g = 0x4, .b = 0x4, .type = TUI_COLOR_RGB };
    row->render.fg_ul = (Tui_Color){ .r = 0x11, .g = 0x11, .b = 0x11, .type = TUI_COLOR_RGB };

    for(size_t i = 0; i < v_ctm_image_length(tm.v_images); ++i) {
        Ctm_Image *image = v_ctm_image_get_at(&tm.v_images, i);
        image->row_owner = row;
        array_push(row->images, image);
    }

    array_push(tm.grid.rows, row);

    tm.grid.dim_cell.y = 5;
    tm.grid.dim_cell.x = 10;
    tm.grid.w_title = 10;

    tm.image_select.render.bg = (Tui_Color){ .r = 0x22, .g = 0x44, .b = 0x25, .type = TUI_COLOR_RGB };

    ctm_loader_image_init(&tm.loader_image, &tm, number_of_processors);

    if(tm.tui_defer) {
        while(tui_core_loop(tm.tui_core)) {}
    }

    pw_cancel(&tm.loader_image.pw);

defer:

    pw_free(&tm.loader_image.pw);

    if(tm.tui_defer) {
        tui_core_free(tm.tui_core);
        tui_exit();
    }

    arg_free(&tm.arg);
    arg_config_free(&tm.arg_config);
    v_ctm_image_free(&tm.v_images);
    ctm_grid_free(&tm.grid);

    return err;
}

