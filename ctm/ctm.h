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
} Ctm_Row, **Ctm_Rows;

typedef struct Ctm_Grid {
    ssize_t h;
    Ctm_Rows rows;
} Ctm_Grid;

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

    size_t              i_image_next;
    size_t              i_image_prev;
    int                 i_image_change;

} Ctm;

#define CTM_H
#endif /* CTM_H */
