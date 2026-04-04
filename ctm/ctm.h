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


typedef struct Ctm_Row {
    So name;
    Tui_Fx fx;
    Tui_Color fg;
    Tui_Color bg;
    Ctm_Image **images;

    struct {
        Tui_Rect rc_row; /* the full width, from left -> right */
        Tui_Rect rc_name;
        Tui_Rect rc_images; /* only up to and including items */
        Tui_Rect rc_bg; /* width from name -> right */
        Tui_Rect rc_ul; /* "" */
        Tui_Color bg_bg;
        Tui_Color fg_ul;
    } render;

} Ctm_Row, **Ctm_Rows;

typedef struct Ctm_Grid {
    Ctm_Rows rows;

    struct {
        Tui_Rect rc_grid;
    } render;
} Ctm_Grid;

typedef struct Ctm_Input {
    int move_x;
    int move_y;
    int select_x;
    int select_y;
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

typedef struct Ctm_Config {
    bool is_graphics_supported;
    Tui_Point dim_cell;
    Tui_Point dim_cell_grab;
    ssize_t w_title;
    VSo categories_use;
    VSo categories_template;
    ssize_t scroll_mult;
    bool scroll_invert;

    ssize_t thumb;
    So argp_dim_cell;
    So argp_dim_cell_grab;
    Color bg_grab;
    Color bg_even;
    Color bg_odd;
    Color fg_ul;
} Ctm_Config;

typedef struct Ctm {

    bool                arg_quit_early;
    struct Arg_Config  *arg_config;
    struct Arg         *arg;

    Ctm_Config          config;
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

} Ctm;

#define CTM_H
#endif /* CTM_H */
