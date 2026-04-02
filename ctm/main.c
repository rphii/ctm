#include <rltui.h>
#include <rlarg.h>
#include <rlso.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize2.h>

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
                    tm->i_image_change = 1;
                    update = true;
                } break;
                case 'k': {
                    tm->i_image_change = -1;
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

bool ctm_update(void *user) {
    Ctm *tm = user;

    bool render = false;

    tm->i_image_prev = tm->i_image_next;

    pthread_mutex_lock(&tm->events.mtx);
    if(tm->events.thumb_loaded) render = true;
    tm->events.thumb_loaded = 0;
    pthread_mutex_unlock(&tm->events.mtx);


    if(tm->i_image_change > 0) {
        ++tm->i_image_next;
        if(tm->i_image_next >= v_ctm_image_length(tm->v_images)) {
            tm->i_image_next = 0;
        }
    } else if(tm->i_image_change < 0) {
        if(tm->i_image_next > 0) {
            --tm->i_image_next;
        } else {
            tm->i_image_next = v_ctm_image_length(tm->v_images) - 1;
        }
    }

    tm->i_image_change = 0;


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

#if 0
    for(size_t i = 0; i < v_ctm_image_length(tm->v_images); ++i) {
        so_clear(&tmp);
        Ctm_Image *image = v_ctm_image_get_at(&tm->v_images, i);

        bool is_loaded = false;
        bool is_valid = false;

        if(!pthread_mutex_trylock(&image->mtx)) {
            is_loaded = image->loaded;
            is_valid = image->data;
            pthread_mutex_unlock(&image->mtx);
        }

        //so_fmt(&tmp, "%.*s : %s : %s", SO_F(image->filename), is_loaded ? "loaded" : "not loaded", is_valid ? "valid" : "not found");

        if(is_valid) {
            image->tui_image->src.dim = image->tui_image->dimensions;
            //image->tui_image->dst.dim = 
            image->tui_image->z = 1;
            tui_image_update(tm->tui_core, image->tui_image, 0);
            tui_image_render(tm->tui_core, image->tui_image, image->unique_id, 0);
        }

        tui_buffer_draw(buffer, rc_images, &fg_images, 0, 0, tmp);
        ++rc_images.anc.y;
    }
    so_free(&tmp);
#endif

    Ctm_Image *image_prev = v_ctm_image_get_at(&tm->v_images, tm->i_image_prev);
    bool is_valid_prev = false;
    if(tm->i_image_next != tm->i_image_prev && !pthread_mutex_trylock(&image_prev->mtx)) {
        is_valid_prev = image_prev->data;
        pthread_mutex_unlock(&image_prev->mtx);
    }

    Ctm_Image *image = v_ctm_image_get_at(&tm->v_images, tm->i_image_next);

    bool is_loaded = false;
    bool is_valid = false;
    if(tm->i_image_next != tm->i_image_prev && !pthread_mutex_trylock(&image->mtx)) {
        is_loaded = image->loaded;
        is_valid = image->data;
        pthread_mutex_unlock(&image->mtx);
    }

    if(is_valid_prev) {
        tui_image_clear_id_place(tm->tui_core, &tmp, image_prev->tui_image->id);
        //printff("cleared prev");
    }
    if(is_valid) {
        image->tui_image->src.dim = image->tui_image->dimensions;
        //image->tui_image->dst.dim = 
        image->tui_image->z = 1;
        tui_image_update(tm->tui_core, image->tui_image, 0);
        tui_image_render(tm->tui_core, image->tui_image, image->unique_id, 0);
        //printff("cleared next");
    }

    so_free(&tmp);

}

void ctm_resized(Tui_Point size, Tui_Point pixels, void *user) {
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

#if 1
    /* start all other ctm structs */
    ctm_loader_image_init(&tm.loader_image, &tm, number_of_processors);
    v_ctm_image_init_from_paths(&tm.v_images, &tm.loader_image, tm.image_paths);
#endif

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

