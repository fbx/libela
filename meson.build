project(
  'ela',
  'c',
  version: '0.3',
  license: 'MIT',
  default_options: [
    'warning_level=2',
    'werror=true',
  ],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
  '-Wstrict-prototypes',
  '-Wmissing-prototypes',
  '-Wno-unused-parameter',
]), language: 'c')

libevent_dep = dependency('libevent')
rt_dep = cc.find_library('rt')

ela_files = []
ela_deps = [
  libevent_dep,
  rt_dep,
]

subdir('include')
subdir('src')

ela_inc = include_directories('include')

lib_ela = library(
  meson.project_name(), ela_files,
  version: meson.project_version(),
  dependencies: ela_deps,
  include_directories: [ela_inc],
  install: true,
)

ela_dep = declare_dependency(
  link_whole: lib_ela,
  include_directories: [ela_inc],
)

if get_option('tests')
  subdir('test')
endif

pkgconfig = import('pkgconfig')
pkgconfig.generate(lib_ela,
  version: meson.project_version(),
  filebase: meson.project_name(),
  name: meson.project_name(),
  description: 'Event loop abstraction library',
)
