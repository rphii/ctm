#include <rltui.h>
#include <rlarg.h>
#include <rlso.h>


#include "ctm.h"
#include "ctm-arg.h"
#include "ctm-image.h"
#include "ctm-loader-image.h"

bool ctm_input(Tui_Input *input, bool *flush, void *user) {
    Ctm *tm = user;
    bool update = false;
    switch(input->id) {
        case INPUT_TEXT: {
            switch(input->text.val) {
                case 'q': {
                    tui_core_quit(tm->tui_core);
                } break;
                case 'j': {
                    update = true;
                } break;
                case 'k': {
                    update = true;
                } break;
            }
        } break;
        case INPUT_CODE: {
        } break;
        case INPUT_MOUSE: {
        } break;
        default: break;
    }
    return update;
}

void ctm_row_image_update(Tui_Point dimensions, Ctm_Grid *grid, Ctm_Row *row, size_t y0, size_t x0) {
    Tui_Rect rc = {0};
    Tui_Point d = (Tui_Point){ .x = grid->h_single_cell * 2, .y = grid->h_single_cell };
    rc.anc = (Tui_Point){ .x = x0, .y = y0 };
    rc.dim = d;
    Ctm_Image **itE = array_itE(row->images);
    for(Ctm_Image **it = row->images; it < itE; ++it) {
        Ctm_Image *image = *it;
        if(rc.anc.y + rc.dim.y > dimensions.y) {
            image->render.rc_image = (Tui_Rect){0};
            continue;
        }
        if(rc.anc.x >= x0 && rc.anc.y >= y0) {
            image->render.rc_image = rc;
        } else {
            image->render.rc_image = (Tui_Rect){0};
        }
        rc.anc.x += d.x;
        if(rc.anc.x + rc.dim.x >= dimensions.x) {
            rc.anc.x = x0;
            rc.anc.y += d.y;
        }
        image->render.is_clean = false;
    }
}

void ctm_row_update(Tui_Point dimensions, Ctm_Grid *grid, Ctm_Row *row, size_t y0) {
    Tui_Point d = dimensions;
    Tui_Rect rc = {0};
    
    /* figure out content rc */
    rc.anc.x = grid->w_title;
    rc.anc.y = y0;
    size_t w_need = array_len(row->images) * grid->h_single_cell * 2;
    size_t w_have = d.x - grid->w_title;
    size_t n_h = (w_need + w_have - 1) / w_have;
    if(!n_h) ++n_h;
    rc.dim.y = n_h * grid->h_single_cell;
    rc.dim.x = w_have;
    row->render.rc_row = rc;

    /* figure out title rc */
    rc.anc.x = 0;
    rc.anc.y = y0;
    rc.dim.y = row->render.rc_row.dim.y;
    rc.dim.x = grid->w_title;
    row->render.rc_name = rc;

    ctm_row_image_update(dimensions, grid, row, y0, row->render.rc_row.anc.x);
}

void ctm_grid_update(Tui_Point dimension, Ctm_Grid *grid) {
    size_t y0 = 0;
    Ctm_Row **itE = array_itE(grid->rows);
    for(Ctm_Row **it = grid->rows; it < itE; ++it) {
        ctm_row_update(dimension, grid, *it, y0);
        y0 += (*it)->render.rc_name.dim.y;
    }
}



bool ctm_update(void *user) {
    Ctm *tm = user;
    bool render = false;

    ctm_grid_update(tm->dimensions, &tm->grid);

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
        tui_buffer_draw(buffer, row->render.rc_row, 0, 0, 0, SO);
        tui_buffer_draw(buffer, row->render.rc_name, &row->fg, &row->bg, &row->fx, row->name);

        Ctm_Image **jtE = array_itE(row->images);
        for(Ctm_Image **jt = row->images; jt < jtE; ++jt) {
            Ctm_Image *image = *jt;
            if(ctm_image_is_valid(image)) {
                /* make sure the image is updated on the TUI side */
                if(!image->render.is_send_error && !image->render.is_send_ok) {
                    image->render.is_send_error = tui_image_update(tm->tui_core, image->tui_image, 0);
                    image->render.is_send_ok = !image->render.is_send_error;
                }
                /* render only if dirty */
                if(!image->render.is_clean) {
                    //printff("place image @ %u,%u",image->render.rc_image.anc.x,image->render.rc_image.anc.y);
                    image->render.is_clean = true;
                    //tui_image_clear_id_place(tm->tui_core, &tmp, image->tui_image->id);
                    image->tui_image->dst = image->render.rc_image;
                    image->tui_image->src.dim = image->tui_image->dimensions;
                    image->tui_image->z = 1;
                    tui_image_render(tm->tui_core, image->tui_image, image->tui_image->id, 0);
                }
            }
        }

    }

    so_free(&tmp);

}

void ctm_resized(Tui_Point size, Tui_Point pixels, void *user) {
    Ctm *tm = user;
    tm->dimensions = size;
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
    }

    /* init all other ctm structs */
    ctm_loader_image_init(&tm.loader_image, &tm, number_of_processors);
    v_ctm_image_init_from_paths(&tm.v_images, &tm.loader_image, tm.image_paths);

    Ctm_Row *row;
    NEW(Ctm_Row, row);
    row->name = so("S");
    row->bg = (Tui_Color){ .r = 0xFF, .g = 0x00, .b = 0x00, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("A");
    row->bg = (Tui_Color){ .r = 0x80, .g = 0x00, .b = 0x00, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("B");
    row->bg = (Tui_Color){ .r = 0x6F, .g = 0x20, .b = 0x00, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("C");
    row->bg = (Tui_Color){ .r = 0x20, .g = 0x60, .b = 0x00, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("D");
    row->bg = (Tui_Color){ .r = 0x00, .g = 0x80, .b = 0x00, .type = TUI_COLOR_RGB };
    array_push(tm.grid.rows, row);
    NEW(Ctm_Row, row);
    row->name = so("E");
    row->bg = (Tui_Color){ .r = 0x00, .g = 0xFF, .b = 0x00, .type = TUI_COLOR_RGB };

    for(size_t i = 0; i < v_ctm_image_length(tm.v_images); ++i) {
        Ctm_Image *image = v_ctm_image_get_at(&tm.v_images, i);
        array_push(row->images, image);
    }

    array_push(tm.grid.rows, row);

    tm.grid.h_single_cell = 5;
    tm.grid.w_title = 10;

    if(tm.tui_defer) {
        while(tui_core_loop(tm.tui_core)) {}
    }

defer:
    if(tm.tui_defer) {
        tui_core_free(tm.tui_core);
        tui_exit();
    }

    arg_free(&tm.arg);
    arg_config_free(&tm.arg_config);
    return err;
}

