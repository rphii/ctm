#ifndef CTM_GRID_H

#include <rltui.h>
#include "ctm-row.h"

typedef struct Ctm_Grid {
    Ctm_Rows rows;

    struct {
        Tui_Rect rc_grid;
    } render;
} Ctm_Grid;

void ctm_grid_update(Tui_Rect rc_grid, Ctm_Config *config, Ctm_Grid *grid);
void ctm_grid_all_dirty(Ctm_Grid *grid);
void ctm_grid_update_dirty(Ctm_Grid *grid, bool unfloat_all, bool unselect_all);
size_t ctm_grid_row_index(Ctm_Grid *grid, Ctm_Row *row);
Ctm_Image *ctm_grid_image_from_pos(Ctm_Grid *grid, Tui_Point pos);
void ctm_grid_change_xy(Ctm_Config *config, Ctm_Grid *grid, Ctm_Image *image, ssize_t change_x, ssize_t change_y, ssize_t *col_i, Ctm_Row **row_change, bool skip_empty);

#define CTM_GRID_H
#endif /* CTM_GRID_H */

