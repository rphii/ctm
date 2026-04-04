#include "ctm-arg.h"

int ctm_arg_parse_category(So so, Color *color, So *name, struct Argx *argx) {

    So rhs, lhs = so_split_ch(so, ':', &rhs);
    rhs = so_trim(rhs);

    *color = (Color){ .r = rand(), .g = rand(), .b = rand() };
    if(so_cmp(lhs, so("random")) && so_as_color(lhs, color)) {
        arg_runtime_set_parse_error_message(argx, "Invalid color : %.*s", SO_F(lhs));
        return -1;
    }
    if(!so_len(rhs)) {
        arg_runtime_set_parse_error_message(argx, "No name specified : %.*s", SO_F(so));
        return -1;
    }


    //printff(" RHS IS: [%.*s]",SO_F(rhs));
    *name = rhs;
    return 0;
}

int ctm_argx_category(struct Argx *argx, void *user, So so) {

    //printff(" CAT : [%.*s]",SO_F(so));

    Color color = {0};
    So name = SO;
    Ctm *tm = user;
    //custom_categories

    int err = ctm_arg_parse_category(so, &color, &name, argx);

    return err;
}

void ctm_arg(Ctm *ctm) {

    struct Arg *arg = ctm->arg;
    struct Argx *x;
    struct Argx_Group *g;

    argx_builtin_env_compgen(arg);
    arg_enable_config_print(arg, true);

    /* positionals */
    x=argx_pos(arg, so("images"), so("input images"));
      argx_type_rest(x, &ctm->image_paths);

    /* builtin stuff */
    g=argx_group(arg, so("core"));
      argx_builtin_opt_help(g, ARGX_BUILTIN_OPT_HELP);
#ifdef CTM_VERSION
      argx_builtin_opt_version(g, ARGX_BUILTIN_OPT_VERSION, so(CTM_VERSION));
#endif
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("/etc/ctm/ctm.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/rphiic/colors.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/ctm/ctm.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$XDG_CONFIG_HOME/ctm/ctm.conf"));

    /* stuff regarding the tier list layout */
    g=argx_group(arg, so("layout"));

    vso_push(&ctm->config.categories_template, so("rgb(ff7f7f):S"));
    vso_push(&ctm->config.categories_template, so("rgb(ffbf7e):A"));
    vso_push(&ctm->config.categories_template, so("rgb(ffdf7e):B"));
    vso_push(&ctm->config.categories_template, so("rgb(ffff80):C"));
    vso_push(&ctm->config.categories_template, so("rgb(cfcfcf):F"));
    x=argx_opt(g, 'c', so("category"), so("specify your own categories\n"
                "* color can be 'random' or a value in the format rgb(RRGGBB)\n"
                "* name can be any string you want for your category\n"
                "* e.g. --color='rgb(ffeeaa):My Category'"));
      argx_type_array_so(x, &ctm->config.categories_use, &ctm->config.categories_template);
      argx_hint_text(x, so("color,name"));
      argx_attr_fatal_config_error(x, true);
      argx_callback(x, ctm_argx_category, ctm, ARGX_PRIORITY_IMMEDIATELY);

    x=argx_opt(g, 'W', so("title-width"), so("set title width"));
      argx_type_size(x, &ctm->config.w_title, &(ssize_t){ 10 });

    x=argx_opt(g, 'x', so("cell-width"), so("set single cell width"));
      argx_type_size(x, &ctm->config.dim_cell.x, &(ssize_t){ 12 });
    x=argx_opt(g, 'y', so("cell-height"), so("set single cell height"));
      argx_type_size(x, &ctm->config.dim_cell.y, &(ssize_t){ 6 });
    x=argx_opt(g, 'G', so("cell-width-grab"), so("set single cell height - grab"));
      argx_type_size(x, &ctm->config.dim_cell_grab.x, &(ssize_t){ 20 });
    x=argx_opt(g, 'g', so("cell-height-grab"), so("set single cell height - grab"));
      argx_type_size(x, &ctm->config.dim_cell_grab.y, &(ssize_t){ 10 });
    x=argx_opt(g, 'X', so("grab-bg"), so("set grab background color"));
      argx_type_color(x, &ctm->config.bg_grab, &(Color){ .r = 0x7f, .g = 0x0, .b = 0x0 });
    x=argx_opt(g, 'u', so("fg-ul"), so("set underline (foreground) color"));
      argx_type_color(x, &ctm->config.fg_ul, &(Color){ .r = 0x11, .g = 0x11, .b = 0x11 });
    x=argx_opt(g, 'e', so("bg-even"), so("set background color of even rows"));
      argx_type_color(x, &ctm->config.bg_even, &(Color){ .r = 0x6, .g = 0x6, .b = 0x6 });
    x=argx_opt(g, 'o', so("bg-odd"), so("set background color of odd rows"));
      argx_type_color(x, &ctm->config.bg_odd, &(Color){ .r = 0x0, .g = 0x0, .b = 0x0 });
    x=argx_opt(g, 's', so("scroll-mult"), so("set background color of odd rows"));
      argx_type_size(x, &ctm->config.scroll_mult, &(ssize_t){ 4 });
    x=argx_opt(g, 'S', so("scroll-invert"), so("set background color of odd rows"));
      argx_type_bool(x, &ctm->config.scroll_invert, &(bool){ false });
    x=argx_opt(g, 't', so("thumb"), so("set thumbnail size ( N*N )"));
      argx_type_size(x, &ctm->config.thumb, &(ssize_t){ 128 });
    x=argx_opt(g, 0, so("random"), so("place tiles randomly"));
      argx_type_bool(x, &ctm->config.random_placement, 0);


    //Color bg_even;
    //Color bg_odd;
    //Color fg_ul;

    argx_builtin_rice(arg);

}

