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
        Tui_Rect rc_name;
        Tui_Rect rc_row;
    } render;

} Ctm_Row, **Ctm_Rows;

typedef struct Ctm_Grid {
    ssize_t w_title;
    ssize_t h_single_cell;
    Ctm_Rows rows;

} Ctm_Grid;

typedef struct Ctm_Input {
    int move_x;
    int move_y;
} Ctm_Input;

typedef struct Ctm {

    bool                arg_quit_early;
    struct Arg_Config  *arg_config;
    struct Arg         *arg;

    VSo                 image_paths;

    bool                tui_defer;
    Tui_Sync            tui_sync;
    struct Tui_Core    *tui_core;
    Tui_Core_Callbacks  tui_core_callbacks;

    Ctm_Loader_Image    loader_image;
    Ctm_Event           events;
    V_Ctm_Image         v_images;

    Ctm_Input           input;
    Ctm_Grid            grid;

    Tui_Point           dimensions;

} Ctm;

#define CTM_H
#endif /* CTM_H */
