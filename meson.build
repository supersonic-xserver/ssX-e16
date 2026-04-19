project('ssX-e16', 'c',
  version: '1.0.17',
  default_options: [
    'warning_level=2',
    'c_std=gnu99',
    'buildtype=debugoptimized',
  ],
)

# Global flags
add_project_arguments('-fcommon', language: 'c')

# Find required packages
x11_dep = dependency('x11')
xext_dep = dependency('xext')
xi_dep = dependency('xi')
xrandr_dep = dependency('xrandr')
xrender_dep = dependency('xrender')
xinerama_dep = dependency('xinerama')
xdamage_dep = dependency('xdamage')
xfixes_dep = dependency('xfixes')
xcomposite_dep = dependency('xcomposite')
xft_dep = dependency('xft', required: get_option('enable-xft'))
imlib2_dep = dependency('imlib2', required: true, method: 'pkg-config')
dbus_dep = dependency('dbus-1', required: get_option('enable-dbus'))

# XPresent extension (check xpresent first, then XPresent)
xpresent_dep = dependency('xpresent', required: false)
if not xpresent_dep.found()
  xpresent_dep = dependency('XPresent', required: false)
endif

# XScreenSaver extension (pkg-config name is xscrnsaver, not xscreensaver)
xscreensaver_dep = dependency('xscrnsaver', required: false)

# Options
enable_sound = get_option('enable-sound')
enable_zoom = get_option('enable-zoom')
enable_glx = get_option('enable-glx')
enable_composite = get_option('enable-composite')
enable_dbus = get_option('enable-dbus')
enable_xft = get_option('enable-xft')
build_modules = get_option('build-modules')

# Configuration data
conf_data = configuration_data()
conf_data.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
conf_data.set_quoted('ENLIGHTENMENT_BIN', join_paths(get_option('prefix'), get_option('bindir')))
conf_data.set_quoted('ENLIGHTENMENT_LIB', join_paths(get_option('prefix'), get_option('libdir'), 'e16'))
conf_data.set_quoted('ENLIGHTENMENT_ROOT', join_paths(get_option('prefix'), get_option('datadir'), 'e16'))
conf_data.set_quoted('PACKAGE', 'ssX-e16')
conf_data.set_quoted('VERSION', '1.0.17')

# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

# Source files
e16_sources = files(
  'src/E.h',
  'src/about.c',
  'src/aclass.c',
  'src/aclass.h',
  'src/actions.c',
  'src/alert.c',
  'src/alert.h',
  'src/animation.c',
  'src/animation.h',
  'src/arrange.c',
  'src/backgrounds.c',
  'src/backgrounds.h',
  'src/borders.c',
  'src/borders.h',
  'src/buttons.c',
  'src/buttons.h',
  'src/comms.c',
  'src/comms.h',
  'src/conf.h',
  'src/config.c',
  'src/container.c',
  'src/container.h',
  'src/coords.c',
  'src/cursors.c',
  'src/cursors.h',
  'src/desktops.c',
  'src/desktops.h',
  'src/dialog.c',
  'src/dialog.h',
  'src/dock.c',
  'src/draw.c',
  'src/econfig.c',
  'src/econfig.h',
  'src/edebug.h',
  'src/edge.c',
  'src/eimage.c',
  'src/eimage.h',
  'src/emodule.c',
  'src/emodule.h',
  'src/eobj.c',
  'src/eobj.h',
  'src/etypes.h',
  'src/events.c',
  'src/events.h',
  'src/ewins.c',
  'src/ewins.h',
  'src/ewin-ops.c',
  'src/ewin-ops.h',
  'src/ewmh.c',
  'src/extinitwin.c',
  'src/file.c',
  'src/file.h',
  'src/finders.c',
  'src/focus.c',
  'src/focus.h',
  'src/fonts.c',
  'src/fx.c',
  'src/grabs.c',
  'src/grabs.h',
  'src/groups.c',
  'src/groups.h',
  'src/handlers.c',
  'src/hints.c',
  'src/hints.h',
  'src/hiwin.c',
  'src/hiwin.h',
  'src/icccm.c',
  'src/iclass.c',
  'src/iclass.h',
  'src/iconify.c',
  'src/icons.c',
  'src/icons.h',
  'src/ipc.c',
  'src/ipc.h',
  'src/lang.c',
  'src/lang.h',
  'src/list.c',
  'src/list.h',
  'src/main.c',
  'src/memory.c',
  'src/menus.c',
  'src/menus.h',
  'src/menus-misc.c',
  'src/misc.c',
  'src/mod-misc.c',
  'src/moveresize.c',
  'src/mwm.c',
  'src/pager.c',
  'src/parse.c',
  'src/parse.h',
  'src/progress.c',
  'src/progress.h',
  'src/regex.c',
  'src/screen.c',
  'src/screen.h',
  'src/session.c',
  'src/session.h',
  'src/settings.c',
  'src/settings.h',
  'src/setup.c',
  'src/shapewin.c',
  'src/shapewin.h',
  'src/size.c',
  'src/slide.c',
  'src/slide.h',
  'src/slideout.c',
  'src/snaps.c',
  'src/snaps.h',
  'src/sounds.h',
  'src/stacking.c',
  'src/startup.c',
  'src/string.c',
  'src/systray.c',
  'src/tclass.c',
  'src/tclass.h',
  'src/text.c',
  'src/theme.c',
  'src/time.c',
  'src/timers.c',
  'src/timers.h',
  'src/tooltips.c',
  'src/tooltips.h',
  'src/user.c',
  'src/user.h',
  'src/util.h',
  'src/warp.c',
  'src/windowmatch.c',
  'src/windowmatch.h',
  'src/xprop.c',
  'src/xprop.h',
  'src/xpropdefs.h',
  'src/x.c',
  'src/xwin.h',
  'src/xtypes.h',
  'src/text_xfs.c',
)

# Conditional sources
if enable_sound
  e16_sources += files('src/sound.c', 'src/sound.h')
endif

if enable_zoom
  e16_sources += files('src/zoom.c')
endif

if enable_glx
  e16_sources += files('src/eglx.c', 'src/eglx.h', 'src/glwin.c')
endif

if enable_composite
  e16_sources += files('src/ecompmgr.c', 'src/ecompmgr.h', 'src/magwin.c')
endif

if enable_dbus
  e16_sources += files('src/edbus.c', 'src/edbus.h')
endif

# Font modules
font_sources = files('src/text_ift.c')
if enable_xft
  font_sources += files('src/text_xft.c')
endif

# Build e16 executable
e16_deps = [
  x11_dep,
  xext_dep,
  xi_dep,
  xrandr_dep,
  xrender_dep,
  xinerama_dep,
  xdamage_dep,
  xfixes_dep,
  xcomposite_dep,
  xft_dep,
  imlib2_dep,
]

if xpresent_dep.found()
  e16_deps += xpresent_dep
endif

if xscreensaver_dep.found()
  e16_deps += xscreensaver_dep
endif

if dbus_dep.found()
  e16_deps += dbus_dep
endif

e16_cargs = [
  '-DLOCALEDIR=' + conf_data.get('LOCALEDIR'),
  '-DENLIGHTENMENT_BIN=' + conf_data.get('ENLIGHTENMENT_BIN'),
  '-DENLIGHTENMENT_LIB=' + conf_data.get('ENLIGHTENMENT_LIB'),
  '-DENLIGHTENMENT_ROOT=' + conf_data.get('ENLIGHTENMENT_ROOT'),
]

e16_libs = []

if imlib2_dep.found()
  e16_libs += imlib2_dep
endif

if xft_dep.found()
  e16_libs += xft_dep
endif

# Subdirectories
subdir('src')
subdir('doc')
subdir('dox')
subdir('eesh')
subdir('epp')
subdir('fonts')
subdir('icons')
subdir('pix')
subdir('scripts')
subdir('config')
subdir('lib')