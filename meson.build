project('nue', 'c', meson_version: '>=1.1', version: '0.0.1')
add_project_arguments('-DCTM_VERSION="'+meson.project_version()+'"', language : 'c')

sources = [
  'ctm/main.c',
  'ctm/ctm-arg.c',
  'ctm/ctm-image.c',
  'ctm/ctm-loader-image.c',
  ]

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)
rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])
rlpw_dep = dependency('rlpw', fallback : ['rlpw', 'rlpw_dep'], default_options: ['default_library=static'])
rlarg_dep = dependency('rlarg', fallback : ['rlarg', 'rlarg_dep'], default_options: ['default_library=static'])
rltui_dep = dependency('rltui', fallback : ['rltui', 'rltui_dep'], default_options: ['default_library=static'])

ctm = executable('ctm',
  sources,
  dependencies: [m_dep, rlc_dep, rlso_dep, rlpw_dep, rlarg_dep, rltui_dep],
  install: true,
  )

install_data('bash/ctm', install_dir: get_option('completion_dir'))

