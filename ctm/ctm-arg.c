#include "ctm-arg.h"

void ctm_arg(Ctm *ctm) {

    struct Arg *arg = ctm->arg;
    struct Argx *x;
    struct Argx_Group *g;

    argx_builtin_env_compgen(arg);
    argx_builtin_rice(arg);

    /* positionals */
    x=argx_pos(arg, so("images"), so("input images"));
      argx_type_rest(x, &ctm->image_paths);

    /* builtin stuff */
    g=argx_group(arg, so("options"));
      argx_builtin_opt_help(g, ARGX_BUILTIN_OPT_HELP);
#ifdef CTM_VERSION
      argx_builtin_opt_version(g, ARGX_BUILTIN_OPT_VERSION, so(CTM_VERSION));
#endif
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("/etc/ctm/ctm.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/rphiic/colors.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$HOME/.config/ctm/ctm.conf"));
      argx_builtin_opt_source(g, ARGX_BUILTIN_OPT_SOURCE, so("$XDG_CONFIG_HOME/ctm/ctm.conf"));

}

