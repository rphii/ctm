#ifndef CTM_IMAGE_H

#include <rlso.h>
#include <rlc.h>
#include <rltui.h>
#include "ctm-config.h"

struct Ctm_Loader_Image;
struct Ctm_Row;
struct Ctm_Grid;

typedef struct Ctm_Image {

    So filename;
    uint8_t *data;
    int width;
    int height;
    int channels;
    bool loaded;
    int unique_id;
    pthread_mutex_t mtx; /* mutex for data */
    Tui_Image *tui_image;
    //Tui_Point coords;

    bool changed_x_index_manually;
    struct Ctm_Row *row_owner;
    So err;

    struct {
        Tui_Rect rc_image_prev;
        Tui_Rect rc_image;
        Tui_Rect rc_cell;
        Tui_Color fallback_bg;
        Tui_Color fallback_fg;
        bool is_send_ok;
        bool is_send_error;
        bool is_render_error;
        bool is_clean; /* if draw ok */
        bool is_floating; /* if grabbed by mouse */
        bool is_selected; /* if selected with kbd mouse */
    } render;

} Ctm_Image;

typedef struct Ctm_Image_LL {
    Ctm_Image *prev;
    Ctm_Image *next;
} Ctm_Image_LL;

void ctm_image_freep(Ctm_Image **image);
void ctm_image_free(Ctm_Image *img);
int ctm_image_cmp(Ctm_Image *a, Ctm_Image *b);

bool ctm_image_is_valid(Ctm_Image *img);
bool ctm_image_is_loaded(Ctm_Image *img);

VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, BASE);
VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, ERR);
VEC_INCLUDE(V_Ctm_Image, v_ctm_image, Ctm_Image, BY_REF, SORT);

void v_ctm_image_init_from_paths(V_Ctm_Image *v, VSo paths);
ATTR_NODISCARD bool ctm_image_unselect(Ctm_Image *image);
ATTR_NODISCARD bool ctm_image_unfloat(Ctm_Image *image);
ATTR_NODISCARD bool ctm_image_unboth(Ctm_Image *image);
size_t ctm_image_get_grid_y(Ctm_Config *config, struct Ctm_Grid *grid, Ctm_Image *image);
Ctm_Image *ctm_image_find_first_best(Ctm_Config *config, struct Ctm_Grid *grid, struct Ctm_Row *row0, Ctm_Image *fallback, int direction);

#define CTM_IMAGE_H
#endif /* CTM_IMAGE_H */

