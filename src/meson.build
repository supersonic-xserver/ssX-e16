# Add build directory to include path for config.h
inc = include_directories('..')

# Build e16 executable with all dependencies
# Note: e16_sources, e16_deps, e16_cargs are defined in root meson.build
e16 = executable('e16', e16_sources,
  c_args: e16_cargs,
  include_directories: inc,
  dependencies: e16_deps,
  link_args: '-lm',
  install: true,
)
