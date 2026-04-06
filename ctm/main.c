#include <rltui.h>
#include <rlarg.h>
#include <rlso.h>

// TODO : rltui -> what if rc changes, and I get an input ?? do I mutex lock the update/input ?

#include "ctm.h"
#include "ctm-arg.h"
#include "ctm-image.h"
#include "ctm-loader-image.h"
#include "ctm-row.h"
#include "ctm-grid.h"

bool ctm_input(Tui_Input *input, bool *flush, void *user) {
    Ctm *tm = user;
    ++tm->n_input;
    Ctm_Input tm_input = tm->input;
    tm->input.input_id = input->id;
    bool update = false;
    switch(input->id) {
        case INPUT_TEXT: {
            switch(input->text.val) {
                case 'q': {
                    tui_core_quit(tm->tui_core);
                } break;
                case 'J': { ++tm->input.move_y; } break;
                case 'K': { --tm->input.move_y; } break;
                case 'L': { ++tm->input.move_x; } break;
                case 'H': { --tm->input.move_x; } break;
                case 'j': { ++tm->input.select_y; } break;
                case 'k': { --tm->input.select_y; } break;
                case 'l': { ++tm->input.select_x; } break;
                case 'h': { --tm->input.select_x; } break;
                case 'X': { tm->input.remove = true; } break;
                case ' ': {
                    tm->input.confirm = true;
                } break;
            }
        } break;
        case INPUT_CODE: {
            if(input->code == KEY_CODE_ENTER) {
                tm->input.next = true;
            }
            if(input->code == KEY_CODE_ESC) {
                tm->input.cancel = true;
            }
        } break;
        case INPUT_MOUSE: {
            tm->input.mouse = input->mouse;
        } break;
        default: break;
    }

    update = memcmp(&tm_input, &tm->input, sizeof(tm_input));

    return update;
}


bool ctm_update(void *user) {
    Ctm *tm = user;
    ++tm->n_update;
    bool render = false;

    bool unfloat_all = false, unselect_all = false;

    if(tm->input.input_id == INPUT_MOUSE) {
        tm->image_select.select.is_kbd = false;
    } else if(tm->input.input_id) {
        tm->image_select.select.is_kbd = true;
    }

    size_t n_rows_total = 0;
    for(size_t i = 0; i < array_len(tm->grid.rows); ++i) {
        Ctm_Row *row = array_at(tm->grid.rows, i);
        n_rows_total += ctm_row_get_rows(&tm->config, row, tm->dimensions.x - tm->config.w_title);
    }

    tm->grid.render.rc_grid.dim.x = tm->dimensions.x;
    tm->grid.render.rc_grid.dim.y = n_rows_total * tm->config.dim_cell.y;
    //printff("n rows %zu = %zu",n_rows_total,tm->grid.render.rc_grid.dim.y);

    if(tm->input.mouse.scroll) {
        ssize_t scroll = tm->config.scroll_invert ? tm->input.mouse.scroll : -tm->input.mouse.scroll;
        scroll *= tm->config.scroll_mult;
        tm->grid.render.rc_grid.anc.y += scroll;
    }

    if(tm->grid.render.rc_grid.anc.y > 0) {
        tm->grid.render.rc_grid.anc.y = 0;
    }
    if(tm->grid.render.rc_grid.dim.y > tm->dimensions.y) {
        if(tm->grid.render.rc_grid.dim.y + tm->grid.render.rc_grid.anc.y < tm->dimensions.y) {
            tm->grid.render.rc_grid.anc.y = -(tm->grid.render.rc_grid.dim.y - tm->dimensions.y);
        }
    } else {
        tm->grid.render.rc_grid.anc.y = 0;
    }

    if(tm->input.mouse.m.press) {
        Ctm_Image *selected = tm->image_select.select.image;
        Ctm_Grid *grid = &tm->grid;
        Ctm_Image *popped = ctm_grid_image_from_pos(grid, tm->input.mouse.pos);
        ctm_row_pop_image(tm, popped, true);

        if(selected == popped) {
            tm->image_select.select.image = 0;
        }
    }

    if(tm->input.mouse.l.press) {

        Ctm_Grid *grid = &tm->grid;
        Ctm_Image *selected = tm->image_select.select.image;
        if(selected) {
            render |= ctm_image_unboth(selected);
        }

        selected = ctm_grid_image_from_pos(grid, tm->input.mouse.pos);
        if(selected) {
            tm->image_select.select.image = selected;

            Tui_Point dg = {
                .x = (tm->config.dim_cell_grab.x - tm->config.dim_cell.x) / 2,
                .y = (tm->config.dim_cell_grab.y - tm->config.dim_cell.y) / 2,
            };

            //selected->render.rc_cell = selected->render.rc_image;
            selected->render.rc_image.dim.x = tm->config.dim_cell_grab.x;
            selected->render.rc_image.dim.y = tm->config.dim_cell_grab.y;
            selected->render.rc_image.anc.x -= dg.x;
            selected->render.rc_image.anc.y -= dg.y;
            
            tm->image_select.select.float_origin = tm->input.mouse.pos;
            tm->image_select.select.float_anc = selected->render.rc_image.anc;

            selected->render.is_floating = true;
            unselect_all = true;
            //selected->render.is_clean = false;
        }

    }


    if(tm->image_select.select.image) {

        Ctm_Image *selected = tm->image_select.select.image;

        if(tm->input.cancel) {
            render |= ctm_image_unboth(selected);
            unfloat_all = true;
        }

        if(tm->input.mouse.l.repeat && selected->render.is_floating) {
            Tui_Point p0 = tm->image_select.select.float_anc;
            Tui_Point p1 = tm->image_select.select.float_origin;
            Tui_Point p2 = tm->input.mouse.pos;
            //Tui_Point pd = (Tui_Point){ .x = selected->render.
            /* new anc = p0 + (p2 - p1) */
            selected->render.rc_image.anc.x = p0.x + (p2.x - p1.x);
            selected->render.rc_image.anc.y = p0.y + (p2.y - p1.y);

            //selected->render.rc_cell.anc.x = p0.x + (p2.x - p1.x);
        }
        if(tm->input.mouse.l.release) {
            if(selected->render.is_floating) {
                render |= ctm_image_unboth(selected);
                unfloat_all = true;
                /* find rect that would be the target for the drop */
                Ctm_Row **itE = array_itE(tm->grid.rows);
                for(Ctm_Row **it = tm->grid.rows; it < itE; ++it) {
                    Ctm_Row *row = *it;
                    Tui_Point ct = (Tui_Point){
                        .x = (selected->render.rc_image.dim.x / 2) + selected->render.rc_image.anc.x,
                        .y = (selected->render.rc_image.dim.y / 2) + selected->render.rc_image.anc.y,
                    };

                    if(tui_rect_encloses_point(row->render.rc_row, ct)) {
                        /* insert into certain position */
                        size_t i = ctm_row_image_index_from_pos(&tm->config, row, ct);
                        ctm_row_image_set(tm, row, selected, i);
                    }
                }
            }
        }
    }

#if 1
    if(tm->image_select.select.is_kbd) {

        Ctm_Image *image = tm->image_select.select.image;

        if(image && tm->input.remove) {
            ctm_row_pop_image(tm, image, true);
            image = 0;
        }

        if(tm->input.next) {
            Ctm_Image *image_old = image;
            image = ctm_image_find_first_best(&tm->config, &tm->grid, 0, image, -1);
            if(image) {
                if(image_old != image) {
                    render |= ctm_image_unboth(image_old);
                }
                tm->image_select.select.image = image;
                image->render.is_selected = true;
            }
        }

        if(!image) {
            image = ctm_image_find_first_best(&tm->config, &tm->grid, 0, image, 1);
            tm->image_select.select.image = image;
        } 

        if(image) {
            if(tm->input.confirm) {
                image->render.is_selected = !image->render.is_selected;
            }

            if(tm->input.select_y || tm->input.select_x) {

                ssize_t i_col = 0;
                Ctm_Row *row_new = 0;
                ctm_grid_change_xy(&tm->config, &tm->grid, image, tm->input.select_x, tm->input.select_y, &i_col, &row_new, true);

                Ctm_Image *image_old = image;
                //printf(TUI_ESC_CODE_GOTO(0,0));
                //printff("new p %p i %zu",row_new,i_col);

                if(i_col >= 0 && i_col < array_len(row_new->images)) {
                    render |= ctm_image_unboth(image_old);
                    image = array_at(row_new->images, i_col);
                    tm->image_select.select.image = image;
                }

                if(image_old != image) {
                    render |= ctm_image_unboth(image_old);
                }
            }

            if(tm->input.move_y || tm->input.move_x) {

                ssize_t i_col = 0;
                Ctm_Row *row_new = 0;
                ctm_grid_change_xy(&tm->config, &tm->grid, image, tm->input.move_x, tm->input.move_y, &i_col, &row_new, false);

                if(row_new != image->row_owner) {
                    image->changed_x_index_manually = false;
                } else {
                    image->changed_x_index_manually = true;
                }

                if(image->changed_x_index_manually) {
                    ctm_row_image_set(tm, row_new, image, i_col);
                } else {
                    ctm_row_image_set(tm, row_new, image, array_len(row_new->images));
                }
            }

            if((tm->input.select_x || tm->input.select_y || tm->input.move_x || tm->input.move_y)) {
                image->render.is_selected = true;
            }

            /* make sure image is visible and not scrolled off */
            if(tm->grid.render.rc_grid.dim.y > tm->dimensions.y) {
                ssize_t y_real = ctm_image_get_grid_y(&tm->config, &tm->grid, image);
                ssize_t y_image = tm->config.dim_cell.y * y_real + tm->config.dim_cell.y / 2;
                ssize_t y_center = tm->dimensions.y / 2;
                ssize_t yd = y_image - y_center;
                tm->grid.render.rc_grid.anc.y = -yd;
            }

        } 
    }

#endif

    ctm_grid_update(tm->grid.render.rc_grid, &tm->config, &tm->grid);

    render |= ctm_grid_update_dirty(&tm->grid, unfloat_all, unselect_all);

    //tm->input_mouse_prev = tm->input.mouse;
    tm->input = (Ctm_Input){0};
    return render;
}

void ctm_render_centered_text(Tui_Buffer *buffer, Ctm *tm, Tui_Rect rc, Tui_Color *fg, Tui_Color *bg, Tui_Fx *fx, So text) {
    So line = SO;
    text = so_trim(text);
    size_t n_lines = so_count_ch(text, '\n') + 1;
    Tui_Rect rc_bx = rc;
    /* just force text to be visible */
    /* top */
    if(rc_bx.anc.y < 0) {
        ssize_t n = rc_bx.anc.y;
        rc_bx.anc.y = 0;
        rc_bx.dim.y += n;
    }
    /* left */
    if(rc_bx.anc.x < 0) {
        ssize_t n = rc_bx.anc.x;
        rc_bx.anc.x = 0;
        rc_bx.dim.x += n;
    }
    /* right */
    if(rc_bx.anc.x + rc_bx.dim.x >= buffer->dimension.x) {
        ssize_t n = buffer->dimension.x - (rc_bx.anc.x + rc_bx.dim.x);
        rc_bx.dim.x += n;
    }
    /* bottm */
    if(rc_bx.anc.y + rc_bx.dim.y >= buffer->dimension.y) {
        ssize_t n = buffer->dimension.y - (rc_bx.anc.y + rc_bx.dim.y);
        rc_bx.dim.y += n;
    }


    if(n_lines < rc_bx.dim.y) {
        rc_bx.anc.y = rc_bx.anc.y + (rc_bx.dim.y - n_lines) / 2;
    }

    /* now actually draw */
    tui_buffer_draw(buffer, rc, fg, bg, fx, SO);
    Tui_Rect rc_tx = rc_bx;
    rc_tx.dim.y = 1;
    while(so_splice(text, &line, '\n') && rc_tx.anc.y < rc_bx.anc.y + rc_bx.dim.y) {
        tui_text_line_clear(&tm->render_tx);
        tui_text_line_fmt(&tm->render_tx, "%.*s", SO_F(line));
        rc_tx.dim.x = tm->render_tx.visual_len;
        if(rc_tx.dim.x > rc_bx.dim.x) rc_tx.dim.x = rc_bx.dim.x;
        if(tm->render_tx.visual_len < rc_bx.dim.x) {
            rc_tx.anc.x = rc_bx.anc.x + (rc_bx.dim.x - tm->render_tx.visual_len) / 2;
        } else {
            rc_tx.anc.x = rc_bx.anc.x;
        }
        //size_t len = tm->render_tx.visual_len;
        //tui_text_line_clear(&tm->render_tx);
        //tui_text_line_fmt(&tm->render_tx, "%zu",len);
        tui_buffer_draw(buffer, rc_tx, fg, bg, fx, tm->render_tx.so);
        ++rc_tx.anc.y;
    }
}


void ctm_render_image(Tui_Buffer *buffer, Ctm_Image *image, ssize_t z) {
    if(!image->render.is_send_error && !image->render.is_send_ok) {
        So err = SO;
        image->render.is_send_error = tui_image_update(buffer, image->tui_image, &err);
        image->render.is_send_ok = !image->render.is_send_error;
        if(image->render.is_send_error) {
            so_copy(&image->err, err);
            //tui_image_clear_all(tm->tui_core);
        }
    }
    /* render only if dirty */
    if(!image->render.is_clean) {
        image->render.rc_image_prev = image->render.rc_image;
        image->render.is_clean = true;
        image->tui_image->dst = image->render.rc_image;
        image->tui_image->src.dim = image->tui_image->dimensions;
        image->tui_image->z = z;
        //printff("Z %i",image->tui_image->z);
        So err = SO;
        image->render.is_render_error = tui_image_render(buffer, image->tui_image, 1, &err);
        if(image->render.is_render_error){
            so_copy(&image->err, err);
            //tui_image_clear_all(tm->tui_core);
        }
    }
}

void ctm_render(Tui_Buffer *buffer, void *user) {


    Ctm *tm = user;
    ++tm->n_render;

    Tui_Color fg_images = { .type = TUI_COLOR_8, .col8 = 2 };
    Tui_Rect rc_images = {
        .anc = (Tui_Point){ .x = 0, .y = 0 },
        .dim = (Tui_Point){ .x = buffer->dimension.x, .y = 1 }
    };

    Tui_Color bg_mono = { .r = tm->config.bg_base.r, .g = tm->config.bg_base.g, .b = tm->config.bg_base.b, .type = TUI_COLOR_RGB };
    tui_buffer_mono(buffer, 0, &bg_mono, 0);

    Tui_Color bg_grab = (Tui_Color){ .r = tm->config.bg_grab.r, .g = tm->config.bg_grab.g, .b = tm->config.bg_grab.b, .type = TUI_COLOR_RGB };

    Ctm_Grid *grid = &tm->grid;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;
        tui_buffer_draw(buffer, row->render.rc_bg, 0, &row->render.bg_bg, 0, SO);
        ctm_render_centered_text(buffer, tm, row->render.rc_name, &row->fg, &row->bg, &row->fx, row->name);
        tui_buffer_draw(buffer, row->render.rc_ul, &row->render.fg_ul, &row->render.bg_bg, 0, tm->render_ul);
    }

    Ctm_Image *on_top = 0;

    bool any_dirty = false;

    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        Ctm_Row *row = *it;

        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;

            if(image->render.is_floating || image->render.is_selected) {

                /* if the cell is grabbed and the grab dimensions < cell dimension, do not draw the background color */
                if(!(tm->image_select.select.is_kbd
                        && image->render.rc_cell.dim.x <= tm->config.dim_cell_grab.x
                        && image->render.rc_cell.dim.y <= tm->config.dim_cell_grab.y)) {
                    tui_buffer_draw(buffer, image->render.rc_cell, 0, &bg_grab, 0, SO);
                }

                ASSERT(on_top == 0, "should only have one on top");
                on_top = image; /* can only have one image on top (atm) :) */
                //continue;
            } else {
                any_dirty |= !image->render.is_clean;
            }

            if(tm->config.is_graphics_supported && ctm_image_is_valid(image) && !image->render.is_render_error && !image->render.is_send_error) {
                ctm_render_image(buffer, image, CTM_IMG_Z_INDEX);
            } else {
                ctm_render_centered_text(buffer, tm, image->render.rc_image, &image->render.fallback_fg, &image->render.fallback_bg, 0, so_get_nodir(image->filename));
            }

        }

    }

    if(on_top) {
        if(tm->config.is_graphics_supported && ctm_image_is_valid(on_top) && !on_top->render.is_render_error && !on_top->render.is_send_error) {
            on_top->render.is_clean = !any_dirty;
            ctm_render_image(buffer, on_top, 1);
        } else {
            ctm_render_centered_text(buffer, tm, on_top->render.rc_image, &on_top->render.fallback_fg, &on_top->render.fallback_bg, 0, so_get_nodir(on_top->filename));
        }
    }

    while(array_len(tm->images_pop)) {
        Ctm_Image *image = array_pop(tm->images_pop);
        tui_image_clear_id_image(buffer, image->tui_image->id);
        ctm_image_free(image);
    }


#if 0
    Tui_Rect rc_stats = {0};
    rc_stats.anc.y = tm->dimensions.y - 3;
    rc_stats.anc.x = tm->dimensions.x - 10;
    rc_stats.dim.y = 3;
    rc_stats.dim.x = 10;
    so_fmt(&tmp, "I:%zu\nU:%zu\nR:%zu", tm->n_input, tm->n_update, tm->n_render);
    tui_buffer_draw(buffer, rc_stats, 0, 0, 0, tmp);
#endif

}

void ctm_resized(Tui_Point size, Tui_Point pixels, void *user) {
    Ctm *tm = user;
    ctm_grid_all_dirty(&tm->grid);
    tm->dimensions = size;

    so_clear(&tm->render_ul);
    for(size_t i = 0; i < size.x - tm->config.w_title; ++i) {
        so_extend(&tm->render_ul, so("▁"));
    }

    //tm->config.is_graphics_supported = true;
}

void ctm_grid_free(Ctm_Grid *grid) {
    array_free_ext(grid->rows, ctm_row_free);
}

int main(int argc, const char **argv) {

    srand(time(0));

    int err = 0;

    Ctm tm = {0};

    /* set up argument config */
    tm.arg_config = arg_config_new(),
    arg_config_set_program(tm.arg_config, so_l(argv[0]));
    arg_config_set_description(tm.arg_config, so("C Tier Maker (create tierlists in your terminal)"));
    arg_config_set_width(tm.arg_config, 100);

    /* set up argument parser */
    tm.arg = arg_new(tm.arg_config);
    ctm_arg(&tm);

    /* parse arguments */
    err = arg_parse(tm.arg, argc, argv, &tm.arg_quit_early);
    if(err || tm.arg_quit_early) goto defer;

    tm.config.is_graphics_supported = !tm.config.no_image;

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
        tm.config.is_graphics_supported &= tui_image_is_supported(tm.tui_core);
    }

    /* init all other ctm structs */

    v_ctm_image_init_from_paths(&tm.v_images, tm.image_paths);

    Ctm_Row *row_last = 0;

    size_t cati = 0;
    So *catE = array_itE(tm.config.categories_use);
    for(So *cat = tm.config.categories_use; cat < catE; ++cat, ++cati) {
        Color color = {0};
        Ctm_Row *row;
        NEW(Ctm_Row, row);
        row_last = row;
        ctm_arg_parse_category(*cat, &color, &row->name, 0);
        row->bg = (Tui_Color){ .r = color.r, .g = color.g, .b = color.b, .type = TUI_COLOR_RGB };
        color.a = 0xff;
        uint8_t brightness = color_as_brightness(color, COLOR_GAMMA_DEFAULT);
        if(brightness) {
            row->fg = (Tui_Color){ .r = 0x00, 0x00, 0x00, TUI_COLOR_RGB };
        } else {
            row->fg = (Tui_Color){ .r = 0xff, 0xff, 0xff, TUI_COLOR_RGB };
        }
        row->render.fg_ul = (Tui_Color){ .r = tm.config.fg_ul.r, .g = tm.config.fg_ul.g, .b = tm.config.fg_ul.b, .type = TUI_COLOR_RGB };
        if(cati % 2) {
            row->render.bg_bg = (Tui_Color){ .r = tm.config.bg_odd.r, .g = tm.config.bg_odd.g, .b = tm.config.bg_odd.b, .type = TUI_COLOR_RGB };
        } else {
            row->render.bg_bg = (Tui_Color){ .r = tm.config.bg_even.r, .g = tm.config.bg_even.g, .b = tm.config.bg_even.b, .type = TUI_COLOR_RGB };
        }
        array_push(tm.grid.rows, row);
    }

    for(size_t i = 0; i < v_ctm_image_length(tm.v_images); ++i) {
        if(!tm.config.random_placement) {
            Ctm_Image *image = v_ctm_image_get_at(&tm.v_images, i);
            image->row_owner = row_last;
            array_push(row_last->images, image);
        } else {
            size_t n_rows = array_len(tm.grid.rows);
            Ctm_Image *image = v_ctm_image_get_at(&tm.v_images, i);
            Ctm_Row *row = array_at(tm.grid.rows, rand() % n_rows);
            image->row_owner = row;
#if 1
            array_push(row->images, image);
#else
            size_t n_images = array_len(row->images) + 1;
            ctm_row_image_set(&tm, row, image, rand() % n_images);
#endif
        }
    }

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

    array_free(tm.images_pop);

    v_ctm_image_free(&tm.v_images);

    ctm_grid_free(&tm.grid);

    so_free(&tm.render_tx.so);
    so_free(&tm.render_ul);

    arg_config_free(&tm.arg_config);
    arg_free(&tm.arg);

    return err;
}

