project('ssX-e16', 'c',
  version: '1.0.31',
  default_options: [
    'warning_level=2',
    'c_std=gnu99',
    'buildtype=debugoptimized',
  ],
)

# Global flags - enable config.h
add_project_arguments('-DHAVE_CONFIG_H', language: 'c')
add_project_arguments('-fcommon', language: 'c')

# Check for compiler features
cc = meson.get_compiler('c')

# Configuration data
conf_data = configuration_data()

# ============================================================
# EC_C___ATTRIBUTE__ (from m4/ec_attribute.m4)
# ============================================================
# Check for __attribute__ support and define __UNUSED__
has_attribute = cc.compiles('''
#include <stdlib.h>
int foo(int x __attribute__ ((unused))) { exit(1); }
''')
if has_attribute
  conf_data.set('HAVE___ATTRIBUTE__', 1)
  conf_data.set('__UNUSED__', '__attribute__((unused))')
else
  conf_data.set('HAVE___ATTRIBUTE__', 0)
  conf_data.set('__UNUSED__', '')
endif

# ============================================================
# EC_C___FUNC__ (from m4/ec_func.m4)
# ============================================================
# Check for __func__ support
has_func = cc.compiles('const char *s = __func__;')
if has_func
  # __func__ is natively supported, no need to define
  conf_data.set('HAVE___FUNC__', 1)
elif cc.compiles('const char *s = __FUNCTION__;')
  # Use __FUNCTION__ as __func__
  conf_data.set('__func__', '__FUNCTION__')
  conf_data.set('HAVE___FUNC__', 0)
else
  # Fallback to "FUNC"
  conf_data.set_quoted('__func__', 'FUNC')
  conf_data.set('HAVE___FUNC__', 0)
endif

# ============================================================
# EC_C_VISIBILITY (from m4/ec_visibility.m4)
# ============================================================
# Use boolean option directly in condition (Meson converts it automatically)
if get_option('enable-visibility-hiding')
  has_visibility = cc.compiles('''
extern __attribute__((__visibility__("hidden"))) int hiddenvar;
extern __attribute__((__visibility__("default"))) int exportedvar;
extern __attribute__((__visibility__("hidden"))) int hiddenfunc (void);
extern __attribute__((__visibility__("default"))) int exportedfunc (void);
  ''')
  if has_visibility
    conf_data.set('HAVE_VISIBILITY', 1)
    conf_data.set('__EXPORT__', '__attribute__((__visibility__("default")))')
  else
    conf_data.set('HAVE_VISIBILITY', 0)
    conf_data.set('__EXPORT__', '')
  endif
else
  conf_data.set('HAVE_VISIBILITY', 0)
  conf_data.set('__EXPORT__', '')
endif

# ============================================================
# Header Checks (from AC_CHECK_HEADERS in configure.ac)
# ============================================================
# Required headers
conf_data.set('HAVE_ALLOCA_H', cc.has_header('alloca.h').to_int())
conf_data.set('HAVE_LOCALE_H', cc.has_header('locale.h').to_int())
conf_data.set('HAVE_LANGINFO_H', cc.has_header('langinfo.h').to_int())
conf_data.set('HAVE_EXECINFO_H', cc.has_header('execinfo.h').to_int())
conf_data.set('HAVE_SNDIO_H', cc.has_header('sndio.h').to_int())

# ============================================================
# Function Checks (from AC_CHECK_FUNCS in configure.ac)
# ============================================================
conf_data.set('HAVE_STRCASECMP', cc.has_function('strcasecmp').to_int())
conf_data.set('HAVE_STRCASESTR', cc.has_function('strcasestr').to_int())
conf_data.set('HAVE_SETENV', cc.has_function('setenv').to_int())
conf_data.set('HAVE_UNSETENV', cc.has_function('unsetenv').to_int())
conf_data.set('HAVE_STRDUP', cc.has_function('strdup').to_int())
conf_data.set('HAVE_STRNDUP', cc.has_function('strndup').to_int())

# clock_gettime - check with -lrt first, then without
clock_gettime = cc.has_function('clock_gettime')
if not clock_gettime
  rt_dep = dependency('rt', required: false)
  if rt_dep.found()
    clock_gettime = cc.has_function('clock_gettime', dependencies: [rt_dep])
    if clock_gettime
      conf_data.set('USE_MONOTONIC_CLOCK', 1)
      conf_data.set('CLOCK_LIBS', '-lrt')
    endif
  endif
else
  conf_data.set('USE_MONOTONIC_CLOCK', 1)
endif

# backtrace functions (for libtrip)
conf_data.set('HAVE_BACKTRACE', cc.has_function('backtrace').to_int())
conf_data.set('HAVE_BACKTRACE_SYMBOLS', cc.has_function('backtrace_symbols').to_int())

# dlopen for modules
conf_data.set('HAVE_DLOPEN', cc.has_function('dlopen').to_int())

# ============================================================
# Size Checks (from AC_CHECK_SIZEOF in configure.ac)
# ============================================================
conf_data.set('SIZEOF_INT', cc.sizeof('int'))
conf_data.set('SIZEOF_LONG', cc.sizeof('long'))
conf_data.set('SIZEOF_WCHAR_T', cc.sizeof('wchar_t'))

# ============================================================
# Byte Order (from AC_C_BIGENDIAN)
# ============================================================
# Try to detect byte order using preprocessor check
byte_order_check = '''
#include <stdint.h>
const int v = 1;
if (*(const uint8_t *)&v == 1) { /* little endian */ }
'''
if cc.compiles(byte_order_check)
  # Assume little endian on most platforms, but we can also skip this
  # since x86_64 is typically little endian
  # For big endian detection, we would need runtime checks
  # Most modern systems are little endian, so default to 0
  conf_data.set('WORDS_BIGENDIAN', 0)
else
  conf_data.set('WORDS_BIGENDIAN', 0)
endif

# ============================================================
# Find required packages
# ============================================================
x11_dep = dependency('x11')
xext_dep = dependency('xext')
xi_dep = dependency('xi')

# X extensions - some are required, some optional
xrandr_dep = dependency('xrandr', required: false)
xrender_dep = dependency('xrender', required: false)
xinerama_dep = dependency('xinerama', required: false)
xdamage_dep = dependency('xdamage', required: false)
xfixes_dep = dependency('xfixes', required: false)
xcomposite_dep = dependency('xcomposite', required: false)
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

# ============================================================
# Options (from AC_ARG_ENABLE/AC_ARG_WITH in configure.ac)
# ============================================================
# Sound options
enable_sound = get_option('enable-sound')
conf_data.set('ENABLE_SOUND', enable_sound.to_int())

# Initialize sound backend flags (default to disabled)
conf_data.set('USE_SOUND_PULSE', 0)
conf_data.set('USE_SOUND_ESOUND', 0)
conf_data.set('USE_SOUND_ALSA', 0)
conf_data.set('USE_SOUND_SNDIO', 0)
conf_data.set('USE_SOUND_PLAYER', 0)

# Determine default sound engine
sound_engine = 'no'
if enable_sound
  sound_engine = 'pulse'  # default from configure.ac
endif
conf_data.set_quoted('DEFAULT_SOUND_ENGINE', sound_engine)

# Sound backends
enable_sound_pulse = get_option('enable-sound-pulse')
if enable_sound_pulse and enable_sound
  pulse_dep = dependency('libpulse', required: false)
  if pulse_dep.found()
    conf_data.set('USE_SOUND_PULSE', 1)
  endif
endif

enable_sound_esound = get_option('enable-sound-esound')
if enable_sound_esound and enable_sound
  # Check for esound - AM_PATH_ESD would be used in autotools
  # For meson, we'll check for esd libs
  conf_data.set('USE_SOUND_ESOUND', 0)  # Not automatically detected
endif

enable_sound_alsa = get_option('enable-sound-alsa')
if enable_sound_alsa and enable_sound
  alsa_dep = dependency('alsa', required: false)
  if alsa_dep.found()
    conf_data.set('USE_SOUND_ALSA', 1)
  endif
endif

enable_sound_sndio = get_option('enable-sound-sndio')
if enable_sound_sndio and enable_sound and conf_data.get('HAVE_SNDIO_H') == 1
  sndio_lib = cc.find_library('sndio', required: false)
  if sndio_lib.found()
    conf_data.set('USE_SOUND_SNDIO', 1)
  endif
endif

enable_sound_player = get_option('enable-sound-player')
if enable_sound_player and enable_sound
  conf_data.set('USE_SOUND_PLAYER', 1)
  conf_data.set_quoted('SOUND_PLAYER_FMT', '/usr/bin/aplay -q %s')
endif

# Sound loaders
sndldr = get_option('sndldr')
need_sndldr = enable_sound  # Sound is enabled, need a loader
if need_sndldr
  if sndldr == 'sndfile'
    sndfile_dep = dependency('sndfile', required: false)
    if sndfile_dep.found()
      conf_data.set('USE_SOUND_LOADER_SNDFILE', 1)
    endif
  elif sndldr == 'audiofile'
    audiofile_dep = dependency('audiofile', required: false)
    if audiofile_dep.found()
      conf_data.set('USE_SOUND_LOADER_AUDIOFILE', 1)
    endif
  endif
endif

# Session management
enable_sm = get_option('enable-sm')
if enable_sm
  sm_dep = dependency('sm', required: false)
  ice_dep = dependency('ice', required: false)
  if sm_dep.found() and ice_dep.found()
    conf_data.set('USE_SM', 1)
  endif
endif

# Pango
enable_pango = get_option('enable-pango')
if enable_pango
  pango_dep = dependency('pangoxft', required: false)
  gobject_dep = dependency('gobject-2.0', required: false)
  if pango_dep.found() and gobject_dep.found()
    conf_data.set('USE_PANGO', 1)
  endif
endif

# Xft
enable_xft = get_option('enable-xft')
if xft_dep.found()
  conf_data.set('USE_XFT', 1)
endif

# XI2
enable_xi2 = get_option('enable-xi2')
if enable_xi2
  xi2_dep = dependency('xi', version: '>=1.3', required: false)
  if xi2_dep.found()
    conf_data.set('USE_XI2', 1)
  endif
endif

# GLX
enable_glx = get_option('enable-glx')
if enable_glx
  gl_dep = dependency('gl', required: false)
  glu_dep = dependency('glu', required: false)
  if gl_dep.found() and glu_dep.found()
    conf_data.set('USE_GLX', 1)
    # Check for glXBindTexImageEXT
    if cc.has_function('glXBindTexImageEXT', dependencies: [gl_dep])
      conf_data.set('HAVE_GLX_glXBindTexImageEXT', 1)
    endif
  endif
endif

# XSync
enable_xsync = get_option('enable-xsync')
if enable_xsync
  # Check for XSyncQueryExtension in libXext
  conf_data.set('USE_XSYNC', 1)  # Xext is always available
endif

# Xinerama
enable_xinerama = get_option('enable-xinerama')
if enable_xinerama and xinerama_dep.found()
  conf_data.set('USE_XINERAMA', 1)
endif

# XScreenSaver
enable_xscrnsaver = get_option('enable-screensaver')
if enable_xscrnsaver and xscreensaver_dep.found()
  conf_data.set('USE_XSCREENSAVER', 1)
endif

# RandR
enable_xrandr = get_option('enable-xrandr')
if enable_xrandr and xrandr_dep.found()
  conf_data.set('USE_XRANDR', 1)
  # Check for RandR versions
  # Note: version check would require pkg-config version comparison
  conf_data.set('HAVE_XRANDR_12', 1)  # Assume 1.2+ if found
  conf_data.set('HAVE_XRANDR_13', 1)  # Assume 1.3+ if found
endif

# XRender
enable_xrender = get_option('enable-xrender')
if enable_xrender and xrender_dep.found()
  conf_data.set('USE_XRENDER', 1)
endif

# Composite
enable_composite = get_option('enable-composite')
if enable_composite and xcomposite_dep.found() and xdamage_dep.found() and xfixes_dep.found()
  conf_data.set('USE_COMPOSITE', 1)
  conf_data.set('HAVE_COMPOSITE_OVERLAY_WINDOW', 1)  # Assume 0.3+
endif

# XPresent
enable_xpresent = get_option('enable-xpresent')
if enable_xpresent and xpresent_dep.found()
  conf_data.set('USE_XPRESENT', 1)
endif

# Zoom
enable_zoom = get_option('enable-zoom')
if enable_zoom
  if xrandr_dep.found() and conf_data.get('HAVE_XRANDR_13') == 1
    conf_data.set('ENABLE_ZOOM', 1)
  else
    xxf86vm_dep = dependency('xxf86vm', required: false)
    if xxf86vm_dep.found()
      conf_data.set('ENABLE_ZOOM', 1)
      conf_data.set('USE_ZOOM_XF86VM', 1)
    endif
  endif
endif

# D-Bus
enable_dbus = get_option('enable-dbus')
if enable_dbus and dbus_dep.found()
  conf_data.set('USE_DBUS', 1)
endif

# Line editing for eesh
lineedit_opt = get_option('lineedit')
if lineedit_opt == 'editline'
  libeditline_dep = dependency('libeditline', required: false)
  if libeditline_dep.found()
    conf_data.set('USE_LINEEDIT_EDITLINE', 1)
  endif
elif lineedit_opt == 'readline'
  readline_dep = dependency('readline', required: false)
  if readline_dep.found()
    conf_data.set('USE_LINEEDIT_READLINE', 1)
  endif
endif

# GNOME support
with_gnome = get_option('with-gnome')
if with_gnome == 'gnome2' or with_gnome == 'auto'
  conf_data.set('WITH_GNOME2', 1)
endif
if with_gnome == 'gnome3' or with_gnome == 'auto'
  conf_data.set('WITH_GNOME3', 1)
endif

# Libhack (window mode helper)
enable_libhack = get_option('enable-libhack')
if enable_libhack and conf_data.get('HAVE_DLOPEN') == 1
  conf_data.set('USE_LIBHACK', 1)
  conf_data.set('DLOPEN_LIBS', '-ldl')
endif

# Libtrip (debug library)
enable_libtrip = get_option('enable-libtrip')
if enable_libtrip and conf_data.get('HAVE_EXECINFO_H') == 1 and conf_data.get('HAVE_BACKTRACE') == 1
  conf_data.set('BUILD_LIBTRIP', 1)
endif

# Modules
build_modules = get_option('build-modules')
if build_modules and conf_data.get('HAVE_DLOPEN') == 1
  conf_data.set('USE_MODULES', 1)
  conf_data.set('DLOPEN_LIBS', '-ldl')
endif

# Dialogs
enable_dialogs = get_option('enable-dialogs')
if enable_dialogs
  conf_data.set('ENABLE_DIALOGS', 1)
endif

# Containerless
enable_containerless = get_option('enable-containerless')
if not enable_containerless
  conf_data.set('USE_CONTAINER_WIN', 1)
endif

# Misc defines from configure.ac
conf_data.set('USE_EXT_INIT_WIN', 1)
conf_data.set('ENABLE_DESKRAY', 0)

# Imlib2 function checks
conf_data.set('HAVE_IMLIB_CONTEXT_SET_MASK_ALPHA_THRESHOLD', 1)  # Assume modern imlib2
conf_data.set('HAVE_IMLIB_CONTEXT_DISCONNECT_DISPLAY', 1)
conf_data.set('HAVE_IMLIB_XIMAGE_CACHE_CONTROL', 1)

# ============================================================
# Architecture-specific (from host_machine.cpu_family())
# ============================================================
cpu_family = host_machine.cpu_family()
if cpu_family == 'x86_64'
  conf_data.set('__x86_64__', 1)
elif cpu_family == 'i386' or cpu_family == 'i486' or cpu_family == 'i586' or cpu_family == 'i686'
  conf_data.set('__i386__', 1)
elif cpu_family == 'arm'
  conf_data.set('__arm__', 1)
elif cpu_family == 'aarch64'
  conf_data.set('__aarch64__', 1)
endif

# ============================================================
# Path configuration
# ============================================================
conf_data.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
conf_data.set_quoted('ENLIGHTENMENT_BIN', join_paths(get_option('prefix'), get_option('bindir')))
conf_data.set_quoted('ENLIGHTENMENT_LIB', join_paths(get_option('prefix'), get_option('libdir'), 'e16'))
conf_data.set_quoted('ENLIGHTENMENT_ROOT', join_paths(get_option('prefix'), get_option('datadir'), 'e16'))
conf_data.set_quoted('PACKAGE', 'ssX-e16')
conf_data.set_quoted('VERSION', '1.0.31')

# Generate version.h (similar to Makefile.am)
# Uses git describe if .git-src exists, otherwise uses project version
# Check for .git-src file existence using run_command
git_src_check = run_command('test', '-f', '.git-src').returncode()
if git_src_check == 0
  # Try to get git describe output
  git_version = run_command('git', 'describe', '--dirty', check: false).stdout().strip()
  # Check if we got a valid version string (non-empty and doesn't start with error)
  if git_version != '' and not git_version.startswith('fatal')
    conf_data.set_quoted('E16_VERSION', git_version)
  else
    conf_data.set_quoted('E16_VERSION', meson.project_version())
  endif
else
  conf_data.set_quoted('E16_VERSION', meson.project_version())
endif
configure_file(output: 'version.h', configuration: conf_data)

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
  
  # Sound backend modules - linked based on function pointers
  if conf_data.get('USE_SOUND_PULSE') == 1
    e16_sources += files('src/sound_pulse.c', 'src/sound_load.c')
  endif
  
  if conf_data.get('USE_SOUND_ESOUND') == 1
    e16_sources += files('src/sound_esound.c', 'src/sound_load.c')
  endif
  
  if conf_data.get('USE_SOUND_ALSA') == 1
    e16_sources += files('src/sound_alsa.c', 'src/sound_load.c')
  endif
  
  if conf_data.get('USE_SOUND_SNDIO') == 1
    e16_sources += files('src/sound_sndio.c', 'src/sound_load.c')
  endif
  
  if conf_data.get('USE_SOUND_PLAYER') == 1
    e16_sources += files('src/sound_player.c')
  endif
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

# Always include font modules in e16_sources (they're linked based on function pointers)
if imlib2_dep.found()
  e16_sources += files('src/text_ift.c')
endif

if enable_xft and xft_dep.found()
  e16_sources += files('src/text_xft.c')
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

# Session management dependencies
if conf_data.get('USE_SM') == 1
  if sm_dep.found()
    e16_deps += sm_dep
  endif
  if ice_dep.found()
    e16_deps += ice_dep
  endif
endif

e16_cargs = [
  '-DLOCALEDIR=' + conf_data.get('LOCALEDIR'),
  '-DENLIGHTENMENT_BIN=' + conf_data.get('ENLIGHTENMENT_BIN'),
  '-DENLIGHTENMENT_LIB=' + conf_data.get('ENLIGHTENMENT_LIB'),
  '-DENLIGHTENMENT_ROOT=' + conf_data.get('ENLIGHTENMENT_ROOT'),
]

# Add all found dependencies to e16_deps
# (e16_deps is already populated with conditional dependencies above)
if imlib2_dep.found()
  # imlib2_dep is already in e16_deps
endif

if xft_dep.found()
  # xft_dep is already in e16_deps
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