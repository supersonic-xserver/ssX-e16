# Add build directory to include path for config.h
inc = include_directories('..')

e16 = executable('e16', e16_sources,
  c_args: e16_cargs,
  include_directories: inc,
  link_args: '-lm',
  install: true,
)
