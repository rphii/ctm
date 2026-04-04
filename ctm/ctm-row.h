#ifndef CTM_ROW_H

#include "ctm-image.h"
#include "ctm-config.h"

struct Ctm_Row;

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

void ctm_row_free(struct Ctm_Row **row);
void ctm_row_image_update(Tui_Rect rc_box, Ctm_Config *config, struct Ctm_Row *row, size_t y0, size_t x0);
size_t ctm_row_get_cols(Ctm_Config *config, struct Ctm_Row *row, ssize_t width);
size_t ctm_row_get_rows(Ctm_Config *config, struct Ctm_Row *row, ssize_t width);
void ctm_row_update(Tui_Rect rc_box, Ctm_Config *config, struct Ctm_Row *row, size_t y0);
size_t ctm_row_image_index(struct Ctm_Row *row, Ctm_Image *image);
size_t ctm_row_image_index_from_pos(Ctm_Config *config, struct Ctm_Row *row, Tui_Point pos);
void ctm_row_pop_image(struct Ctm *tm, Ctm_Image *image, bool do_delete);
void ctm_row_image_set(struct Ctm *tm, struct Ctm_Row *row, Ctm_Image *image, size_t i);

#define CTM_ROW_H
#endif /* CTM_ROW_H */

