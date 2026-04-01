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
    Ctm *ctm = user;

    bool render = false;

    pthread_mutex_lock(&ctm->events.mtx);
    if(ctm->events.thumb_loaded) render = true;
    ctm->events.thumb_loaded = 0;
    pthread_mutex_unlock(&ctm->events.mtx);

    return render;
}



void display_image(Ctm_Image *image) {

    int xnew = image->width;
    int ynew = image->height;

    //unsigned char *cdata2 = malloc(xnew * ynew * image->channels);
    //stbir_resize_uint8_linear(image->data, image->width, image->height, 0, cdata2, xnew, ynew, 0, image->channels);
    uint8_t *cdata2 = image->data;

    So data = so_ll(cdata2, xnew * ynew * image->channels);
    //printff("read data, encode b64");

    So base64 = SO;
    so_base64_fmt_encode(&base64, data);
    //printff("encoded b64, chunk");

    So display = so("\e_G");
    //so_fmt(&display, "a=T,f=%u,s=%u,v=%u", task->i, ch*8, x, y);

    so_fmt(&display, "a=T,X=%u,Y=%u,c=%u,f=%u,s=%u,v=%u", 0, 0, 0, image->channels*8, xnew, ynew);

    //so_fmt(&display, "a=T,x=%u,y=%u,f=%u,s=%u,v=%u", 0, 0, ch*8, xnew, ynew);
    //so_fmt(&display, "q=1,i=%u,f=%u,s=%u,v=%u", task->i, ch*8, x, y);
    unsigned int chunk = 4000;
    if(so_len(base64) > chunk) {
        for(size_t i = 0; i < so_len(base64); i += chunk) {
            bool more =  i + chunk < so_len(base64);
            if(!i) {
                so_fmt(&display, ",m=1;");
            } else {
                so_fmt(&display, "\e\\\e_Gm=%u;", more);
            }
            if(more) {
                So sub = so_sub(base64, i, i + chunk + 1);
                so_extend(&display, sub);
            } else {
                So sub = so_i0(base64, i);
                so_extend(&display, sub);
            }
        }
    } else {
        so_fmt(&display, ";");
        so_extend(&display, base64);
    }
    so_fmt(&display, "\e\\");
    so_print(display);
    so_free(&display);

}


void ctm_render(Tui_Buffer *buffer, void *user) {
    Ctm *ctm = user;

    So tmp = SO;
    Tui_Color fg_images = { .type = TUI_COLOR_8, .col8 = 2 };
    Tui_Rect rc_images = {
        .anc = (Tui_Point){ .x = 0, .y = 0 },
        .dim = (Tui_Point){ .x = buffer->dimension.x, .y = 1 }
    };
    for(size_t i = 0; i < v_ctm_image_length(ctm->v_images); ++i) {
        so_clear(&tmp);
        Ctm_Image *image = v_ctm_image_get_at(&ctm->v_images, i);

        bool is_loaded = false;
        bool is_valid = false;

        if(!pthread_rwlock_tryrdlock(&image->rwlock)) {
            is_loaded = image->loaded;
            is_valid = image->data;
            pthread_rwlock_tryrdlock(&image->rwlock);
        }

        display_image(image);

        so_fmt(&tmp, "%.*s : %s : %s", SO_F(image->filename), is_loaded ? "loaded" : "not loaded", is_valid ? "valid" : "not found");

        tui_buffer_draw(buffer, rc_images, &fg_images, 0, 0, tmp);
        ++rc_images.anc.y;
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

    /* start all other ctm structs */
    ctm_loader_image_init(&tm.loader_image, &tm, number_of_processors);
    v_ctm_image_init_from_paths(&tm.v_images, &tm.loader_image, tm.image_paths);

    /* set up the tui portion of tui */
    tm.tui_defer = true;
    if(tm.tui_defer) {
        tm.tui_core = tui_core_new();
        tm.tui_core_callbacks.input = ctm_input;
        tm.tui_core_callbacks.update = ctm_update;
        tm.tui_core_callbacks.render = ctm_render;
        tm.tui_core_callbacks.resized = ctm_resized;
        tui_core_init(tm.tui_core, &tm.tui_core_callbacks, &tm.tui_sync, &tm);

        tui_enter();
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

