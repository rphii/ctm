#ifndef CTM_CONFIG_H

#include <rlso.h>
#include <rltui.h>

typedef struct Ctm_Config {
    bool is_graphics_supported;
    Tui_Point dim_cell;
    Tui_Point dim_cell_grab;
    ssize_t w_title;
    VSo categories_use;
    VSo categories_template;
    ssize_t scroll_mult;
    bool scroll_invert;
    bool random_placement;
    bool custom_categories;
    bool no_image;

    Tui_Point dim_cell_grab_fast;

    ssize_t thumb;
    So argp_dim_cell;
    So argp_dim_cell_grab;
    Color bg_grab;
    Color bg_even;
    Color bg_odd;
    Color fg_ul;
    Color bg_base;
} Ctm_Config;

#define CTM_CONFIG_H
#endif /* CTM_CONFIG_H */

