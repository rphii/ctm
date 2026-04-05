#ifndef CTM_H

#include <stdbool.h>

#include <rlarg.h>
#include <rlc.h>
#include <rlpw.h>
#include <rlso.h>
#include <rltui.h>

#include "ctm-loader-image.h"
#include "ctm-event.h"
#include "ctm-image.h"
#include "ctm-config.h"
#include "ctm-grid.h"

#define CTM_IMG_Z_INDEX      (INT32_MIN/2)


typedef struct Ctm_Input {
    int move_x;
    int move_y;
    int select_x;
    int select_y;
    bool cancel;
    bool confirm;
    bool next;
    Tui_Input_List input_id;
    Tui_Mouse mouse;
} Ctm_Input;

typedef struct Ctm_Image_Select {

    struct {
        Ctm_Image *image;
        Tui_Point float_anc;
        Tui_Point float_origin;
        bool is_kbd;
    } select;

} Ctm_Image_Select;

typedef struct Ctm {

    bool load_img;

    size_t n_input;
    size_t n_render;
    size_t n_update;

    bool                arg_quit_early;
    struct Arg_Config  *arg_config;
    struct Arg         *arg;

    Ctm_Config          config;
    Ctm_Config          config_fb;
    VSo                 image_paths;

    bool                tui_defer;
    Tui_Sync            tui_sync;
    struct Tui_Core    *tui_core;
    Tui_Core_Callbacks  tui_core_callbacks;

    Ctm_Loader_Image    loader_image;
    Ctm_Event           events;
    V_Ctm_Image         v_images;

    //Tui_Mouse           input_mouse_prev;
    Ctm_Input           input;
    Ctm_Image_Select    image_select;
    Ctm_Grid            grid;

    Tui_Point           dimensions;
    So                  render_tmp;
    So                  render_ul;

} Ctm;

#define CTM_H
#endif /* CTM_H */
