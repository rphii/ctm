# ctm

**ctm: C Tier Maker (create tierlists in your terminal)***

## compile, install

```sh
git clone --recursive https://github.com/rphii/ctm && cd ctm
meson setup build --buildtype release
meson compile -C build  # you can skip this if you only care about installing
meson install -C build
```

- binary is in `build/ctm`. 
- there is a [bash auto comletion](bash/ctm) script (that relies on `rlarg`) - it gets installed into [`/usr/share/bash-completion/completions/`](meson.options)

### dependencies (pulled in by meson, or git)

- [nothings/stb](https://github.com/nothings/stb) images
- [rphii/rlc](https://github.com/rphii/rlc) core helpers
- [rphii/rlso](https://github.com/rphii/rlso) string object
- [rphii/rlpw](https://github.com/rphii/rlpw) parallel worker
- [rphii/rltui](https://github.com/rphii/rltui) tui library
- [rphii/rlarg](https://github.com/rphii/rlarg) argument parser

### image support

I tested image support in [kitty](https://sw.kovidgoyal.net/kitty/), [konsole](https://apps.kde.org/konsole/) and [ghostty](https://ghostty.org/)

- kitty, konsole: ok
- ghostty: image support is detected, but it fails [here](https://github.com/ghostty-org/ghostty/blob/ba398dfff3e30ff83da07140981ca138410cf608/src/terminal/kitty/graphics_command.zig#L220)
    (ghostty bugs out completely at times, I want to fix it, but I also want to release ctm now)

If image support is detected to be not present, i.e. other terminals, [st](https://st.suckless.org/) it will still display the name.txt

## usage

- general help, etc: `ctm -h`

- launch to see layout: `ctm`
- launch without displaying images `ctm -N assets/*`
- launch only using arch and gentoo `ctm -N assets/arch.png assets/gentoo.png`
- launch using images `ctm assets/*`
- launch using the supplied green [`conf`](conf): `ctm --source conf assets/*`
- launch with anything: `ctm 'word 1' 'word 2'`
- launch with wild names: `ctm -- your wild names go -here-`

- list available template config `ctm -h environment.CONFIG_PRINT`
- print template config `CONFIG_PRINT=layout,core ctm`

- `ctm` does not go through files recursively _(yet)_

### hotkeys

- `q`  quit
- `hjkl` select left,down,up,right
- `HJKL` move left,down,up,right
- `X` remove
- `<ESC>` cancel selection
- `<SPACE>` toggle selection
- `<ENTER>` select first item in lowest row

### mouse

- drag and drop tiles
- middle click to remove
- scroll to change view

