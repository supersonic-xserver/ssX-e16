AC_INIT([e16],[1.0.26],[enlightenment-devel@lists.sourceforge.net])
AM_INIT_AUTOMAKE([foreign dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CC
AM_PROG_CC_C_O

AC_PROG_INSTALL

PKG_PROG_PKG_CONFIG

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_GCJ_CONFIG], [:])dnl
AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT([dlopen])

AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADERS(alloca.h locale.h langinfo.h)

AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
EC_C___ATTRIBUTE__
EC_C___FUNC__
EC_C_VISIBILITY(yes)

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(wchar_t, 4)

AC_CHECK_FUNCS(strcasecmp strcasestr)
AC_CHECK_FUNCS(setenv unsetenv)
AC_CHECK_FUNCS(strdup strndup)

AC_CHECK_FUNCS(blumfrub)
AC_CHECK_FUNCS(buckets_of_erogenous_nym)
AC_CHECK_FUNCS(buttox)

AM_ICONV

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19])
m4_ifdef([AM_GNU_GETTEXT_REQUIRE_VERSION], [
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19])
])

AC_CHECK_FUNCS([clock_gettime], [have_clock_gettime=yes],
  [AC_CHECK_LIB([rt], [clock_gettime], [have_clock_gettime=-lrt],
     [have_clock_gettime=no])])
if test "x$have_clock_gettime" != xno; then
  AC_DEFINE(USE_MONOTONIC_CLOCK, 1, [Use monotonic clock from clock_gettime()])
  if test "x$have_clock_gettime" != xyes; then
    AC_SUBST(CLOCK_LIBS, [$have_clock_gettime])
  fi
fi

AC_ARG_WITH(evhan,
  AS_HELP_STRING([--with-evhan],
                 [select event handling api (poll/select)@<:@default=auto@:>@]),,
                 AC_CHECK_HEADERS(poll.h, with_evhan=poll, with_evhan=select))
case "$with_evhan" in
poll)
  AC_CHECK_HEADERS(poll.h,, AC_MSG_ERROR([Need poll.h]))
  AC_DEFINE(USE_EVHAN_POLL, 1, [Use poll as event handler api])
  ;;
select)
  AC_DEFINE(USE_EVHAN_SELECT, 1, [Use select as event handler api])
  ;;
*)
  AC_MSG_ERROR([Invalid option: --with-evhan=$with_evhan])
  ;;
esac

AC_ARG_ENABLE(sound,
  AS_HELP_STRING([--enable-sound],
                 [compile with sound support (select default)  (yes/no/pulseaudio/esound/sndio/alsa/player)@<:@default=yes@:>@]),
  enable_sound="$enableval",
  enable_sound="yes")
case "$enable_sound" in
yes | pulse*)
        enable_sound_pulse="yes"; enable_sound="pulse" ;;
esound) enable_sound_esound="yes" ;;
alsa)   enable_sound_alsa="yes" ;;
sndio)  enable_sound_sndio="yes" ;;
player) enable_sound_player="yes" ;;
*)
  enable_sound="no"
  enable_sound_pulse="no"
  enable_sound_esound="no"
  enable_sound_alsa="no"
  enable_sound_sndio="no"
  enable_sound_player="no"
  ;;
esac
default_sound_engine="$enable_sound"

if test "x$enable_sound" != "xno"; then
  AC_DEFINE(ENABLE_SOUND, 1, [Sound support])
  AC_DEFINE_UNQUOTED(DEFAULT_SOUND_ENGINE, "$default_sound_engine", [Default sound engine])
  enable_sound="yes"
fi
AM_CONDITIONAL(ENABLE_SOUND, test "x$enable_sound" = "xyes")

AC_ARG_ENABLE(sound_pulse,
  AS_HELP_STRING([--enable-sound-pulse],
                 [Enable PulseAudio sound support @<:@default=no@:>@]),
  enable_sound_pulse="$enableval",
  enable_sound_pulse="no")
if test "x$enable_sound_pulse" = "xyes"; then
  PKG_CHECK_MODULES(PULSE, libpulse,
    AC_DEFINE(USE_SOUND_PULSE, 1, [PulseAudio sound support])
    need_sndldr="yes",
    enable_sound_pulse="no")
fi
AM_CONDITIONAL(USE_SOUND_PULSE, test "x$enable_sound_pulse" = "xyes")

AC_ARG_ENABLE(sound_esound,
  AS_HELP_STRING([--enable-sound-esound],
                 [Enable EsounD sound support @<:@default=no@:>@]),
  enable_sound_esound="$enableval",
  enable_sound_esound="no")
if test "x$enable_sound_esound" = "xyes"; then
  AM_PATH_ESD(0.2.17,,[
    enable_sound_esound=no
    AC_MSG_WARN([EsounD sound support was requested but not found.])
  ])
  if test "x$enable_sound_esound" = "xyes"; then
    AC_DEFINE(USE_SOUND_ESOUND, 1, [EsounD sound support])
    need_sndldr="yes"
  fi
fi
AM_CONDITIONAL(USE_SOUND_ESOUND, test "x$enable_sound_esound" = "xyes")

AC_ARG_ENABLE(sound_alsa,
  AS_HELP_STRING([--enable-sound-alsa],
                 [Enable ALSA sound support @<:@default=no@:>@]),
  enable_sound_alsa="$enableval",
  enable_sound_alsa="no")
if test "x$enable_sound_alsa" = "xyes"; then
  PKG_CHECK_MODULES(ALSA, alsa,
    AC_DEFINE(USE_SOUND_ALSA, 1, [ALSA sound support])
    need_sndldr="yes",
    enable_sound_alsa="no")
fi
AM_CONDITIONAL(USE_SOUND_ALSA, test "x$enable_sound_alsa" = "xyes")

AC_ARG_ENABLE(sound_sndio,
  AS_HELP_STRING([--enable-sound-sndio],
                 [Enable Sndio sound support @<:@default=no@:>@]),
  enable_sound_sndio="$enableval",
  enable_sound_sndio="no")
if test "x$enable_sound_sndio" = "xyes"; then
  AC_CHECK_HEADERS(sndio.h,, enable_sound_sndio=no
    AC_MSG_WARN([sndio sound support was requested but not found.]))
  AC_CHECK_LIB(sndio, sio_open, SNDIO_LIBS="-lsndio", enable_sound_sndio="no")
  AC_SUBST(SNDIO_LIBS)
  if test "x$enable_sound_sndio" = "xyes"; then
    AC_DEFINE(USE_SOUND_SNDIO, 1, [Sndio sound support])
    need_sndldr="yes"
  fi
fi
AM_CONDITIONAL(USE_SOUND_SNDIO, test "x$enable_sound_sndio" = "xyes")

AC_ARG_ENABLE(sound_player,
  AS_HELP_STRING([--enable-sound-player],
                 [Enable playing sounds using audio player @<:@default=yes@:>@]),
  enable_sound_player="$enableval",
  enable_sound_player="yes")
AC_ARG_WITH(sound_player,
  AS_HELP_STRING([--with-sound-player],
                 [Audio player @<:@default="/usr/bin/aplay -q %s"@:>@]),
  with_sound_player="$enableval",
  with_sound_player="/usr/bin/aplay -q %s")
if test "x$enable_sound_player" = "xyes"; then
  AC_DEFINE(USE_SOUND_PLAYER, 1, [Play sounds using audio player])
  AC_DEFINE_UNQUOTED(SOUND_PLAYER_FMT, "$with_sound_player", [Audio player])
fi
AM_CONDITIONAL(USE_SOUND_PLAYER, test "x$enable_sound_player" = "xyes")

AC_ARG_WITH(sndldr,
  AS_HELP_STRING([--with-sndldr],
                 [select sound loader (audiofile/sndfile/none) @<:@default=sndfile@:>@]),,
  with_sndldr=sndfile)

if test "x$with_sndldr" = "xsndfile"; then
  PKG_CHECK_MODULES(SNDFILE, sndfile,
    AC_DEFINE(USE_SOUND_LOADER_SNDFILE, 1, [Use sndfile sound loader]),
    with_sndldr=none)
fi
if test "x$with_sndldr" = "xaudiofile"; then
  PKG_CHECK_MODULES(AUDIOFILE, audiofile,
    AC_DEFINE(USE_SOUND_LOADER_AUDIOFILE, 1, [Use audiofile sound loader]),
    with_sndldr=none)
fi

if test "x$need_sndldr" = "xyes"; then
  if test "x$with_sndldr" = "xnone"; then
    AC_MSG_ERROR([Sound support requires a sound loader])
  fi
fi
AM_CONDITIONAL(USE_SOUND_LOADER, test "x$need_sndldr" = "xyes")

# Save CPPFLAGS/LDFLAGS and add X_... to each
SAVE_CPPFLAGS="$CPPFLAGS"
SAVE_LDFLAGS="$LDFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"

PKG_CHECK_MODULES(IMLIB2, imlib2 >= 1.2.0, have_good_imlib2=yes)
AC_CHECK_LIB(Imlib2, imlib_context_set_mask_alpha_threshold,
  AC_DEFINE(HAVE_IMLIB_CONTEXT_SET_MASK_ALPHA_THRESHOLD, 1,
            [Imlib2 has imlib_context_set_mask_alpha_threshold]),,
  $IMLIB2_LIBS)
AC_CHECK_LIB(Imlib2, imlib_context_disconnect_display,
  AC_DEFINE(HAVE_IMLIB_CONTEXT_DISCONNECT_DISPLAY, 1,
            [Imlib2 has imlib_context_disconnect_display]),,
  $IMLIB2_LIBS)
AC_CHECK_LIB(Imlib2, imlib_set_ximage_cache_count_max,
  AC_DEFINE(HAVE_IMLIB_XIMAGE_CACHE_CONTROL, 1,
            [Imlib2 has XImage cache control functions]),,
  $IMLIB2_LIBS)

AC_ARG_ENABLE(sm,
  AS_HELP_STRING([--enable-sm], [compile with session management support @<:@default=yes@:>@]),,
  enable_sm=yes)
if test "x$enable_sm" = "xyes"; then
  PKG_CHECK_MODULES(SM, sm ice, ,
    enable_sm=no
    AC_MSG_WARN([Session management support was requested but not found]) )
fi
if test "x$enable_sm" = "xyes"; then
  AC_DEFINE(USE_SM, 1, [Session management support])
fi

AC_ARG_ENABLE(pango,
  AS_HELP_STRING([--enable-pango], [compile with pango-xft support @<:@default=yes@:>@]),,
  enable_pango=yes)
if test "x$enable_pango" = "xyes"; then
  PKG_CHECK_MODULES(PANGO, pangoxft gobject-2.0, AC_DEFINE(USE_PANGO, 1, [pango-xft support]), enable_pango=no)
fi
AM_CONDITIONAL(USE_LIBPANGO, test "x$enable_pango" = "xyes")

AC_ARG_ENABLE(xft,
  AS_HELP_STRING([--enable-xft], [compile with Xft support @<:@default=yes@:>@]),,
  enable_xft=yes)
if test "x$enable_xft" = "xyes"; then
  PKG_CHECK_MODULES(XFT, xft, AC_DEFINE(USE_XFT, 1, [Xft support]), enable_xft=no)
fi
AM_CONDITIONAL(USE_LIBXFT, test "x$enable_xft" = "xyes")

AC_ARG_ENABLE(xi2,
  AS_HELP_STRING([--enable-xi2], [compile with XI2 support (experimental) @<:@default=no@:>@]),,
  enable_xi2=no)
if test "x$enable_xi2" = "xyes"; then
  PKG_CHECK_MODULES(XI, xi >= 1.3, AC_DEFINE(USE_XI2, 1, [XI2 support]), enable_xi2=no)
fi

AC_ARG_ENABLE(glx,
  AS_HELP_STRING([--enable-glx], [compile with GLX support (experimental) @<:@default=no@:>@]),,
  enable_glx=no)
if test "x$enable_glx" = "xyes"; then
  PKG_CHECK_MODULES(GLX, gl glu,
    AC_DEFINE(USE_GLX, 1, [GLX support])
    AC_CHECK_LIB(GL, glXBindTexImageEXT,
      AC_DEFINE(HAVE_GLX_glXBindTexImageEXT, 1, [glXBindTexImageEXT is available]),
      GLX_LIBS="$GLX_LIBS -ldl"),
    enable_glx=no
    AC_MSG_WARN([GL support was requested but not found]) )
fi
AM_CONDITIONAL(ENABLE_GLX, test "x$enable_glx" = "xyes")

AC_CHECK_LIB(Xext, XShapeQueryExtension,
  E_X_LIBS="-lXext $E_X_LIBS",
  AC_MSG_ERROR([Shape support is required but was not found]),
  -lX11)

AC_ARG_ENABLE(xsync,
  AS_HELP_STRING([--enable-xsync], [compile with SYNC support @<:@default=yes@:>@]),,
  enable_xsync=yes)
if test "x$enable_xsync" = "xyes"; then
  AC_CHECK_LIB(Xext, XSyncQueryExtension,
    AC_DEFINE(USE_XSYNC, 1, [SYNC support]),
    enable_xsync=no
    AC_MSG_WARN([Sync support was requested but not found]),
    -lX11)
fi

AC_ARG_ENABLE(xinerama,
  AS_HELP_STRING([--enable-xinerama], [compile with xinerama support @<:@default=yes@:>@]),,
  enable_xinerama=yes)
if test "x$enable_xinerama" = "xyes"; then
  PKG_CHECK_MODULES(XINERAMA, xinerama,
    AC_DEFINE(USE_XINERAMA, 1, [Xinerama support]),
    enable_xinerama=no
    AC_MSG_WARN([Xinerama support was requested but not found]) )
fi

AC_ARG_ENABLE(xscrnsaver,
  AS_HELP_STRING([--enable-xscrnsaver],
                 [compile with ScreenSaver support (experimental/not useful) @<:@default=no@:>@]),,
  enable_xscrnsaver=no)
if test "x$enable_xscrnsaver" = "xyes"; then
  PKG_CHECK_MODULES(XSCREENSAVER, xscrnsaver,
    AC_DEFINE(USE_XSCREENSAVER, 1, [ScreenSaver support]),
    enable_xscrnsaver=no
    AC_MSG_WARN([ScreenSaver support was requested but not found]) )
fi

AC_ARG_ENABLE(xrandr,
  AS_HELP_STRING([--enable-xrandr], [compile with RandR support @<:@default=yes@:>@]),,
  enable_xrandr=yes)
if test "x$enable_xrandr" = "xyes"; then
  PKG_CHECK_MODULES(XRANDR, xrandr,
    AC_DEFINE(USE_XRANDR, 1, [RandR support])
    PKG_CHECK_EXISTS(xrandr >= 1.2, AC_DEFINE(HAVE_XRANDR_12, 1, [RandR 1.2 support])
      have_randr_12=y)
    PKG_CHECK_EXISTS(xrandr >= 1.3, AC_DEFINE(HAVE_XRANDR_13, 1, [RandR 1.3 support])
      have_randr_13=y),
    enable_xrandr=no
    AC_MSG_WARN([RandR support was requested but not found]) )
fi

AC_ARG_ENABLE(xrender,
  AS_HELP_STRING([--enable-xrender], [compile with Render support @<:@default=yes@:>@]),,
  enable_xrender=yes)
if test "x$enable_xrender" = "xyes"; then
  PKG_CHECK_MODULES(XRENDER, xrender,
    AC_DEFINE(USE_XRENDER, 1, [Render support]),
    enable_xrender=no
    AC_MSG_WARN([Render support was requested but not found]) )
fi

AC_ARG_ENABLE(composite,
  AS_HELP_STRING([--enable-composite], [compile with Composite support @<:@default=yes@:>@]),,
  enable_composite=yes)
if test "x$enable_xrender" != "xyes"; then enable_composite=no; fi
if test "x$enable_composite" = "xyes"; then
  PKG_CHECK_MODULES(XCOMPOSITE, xcomposite,, enable_composite=no)
  PKG_CHECK_MODULES(XDAMAGE, xdamage,, enable_composite=no)
  PKG_CHECK_MODULES(XFIXES, xfixes,, enable_composite=no)
  if test "x$enable_composite" = "xyes"; then
    AC_DEFINE(USE_COMPOSITE, 1, [Composite support])
    PKG_CHECK_EXISTS(xcomposite >= 0.3,
      AC_DEFINE(HAVE_COMPOSITE_OVERLAY_WINDOW, 1, [Composite 0.3 support]))
  else
    AC_MSG_WARN([Composite support was requested but required component was not found])
  fi
fi
AM_CONDITIONAL(ENABLE_COMPOSITE, test "x$enable_composite" = "xyes")

AC_ARG_ENABLE(xpresent,
  AS_HELP_STRING([--enable-xpresent], [compile with Present support (experimental) @<:@default=no@:>@]),,
  enable_xpresent=no)
if test "x$enable_xpresent" = "xyes"; then
  PKG_CHECK_MODULES(XPRESENT, xpresent,
    AC_DEFINE(USE_XPRESENT, 1, [Present support]),
    enable_xpresent=no
    AC_MSG_WARN([Present support was requested but not found]) )
fi

AC_ARG_ENABLE(zoom,
  AS_HELP_STRING([--enable-zoom], [compile with zoom support @<:@default=yes@:>@]),,
  enable_zoom=yes)
if test "x$enable_zoom" = "xyes"; then
  if test "x$have_randr_13" = "xy"; then
    enable_zoom=xrandr
  else
    enable_zoom=xf86vm
  fi
fi
if test "x$enable_zoom" = "xxf86vm"; then
  PKG_CHECK_MODULES(XXF86VM, xxf86vm,
    AC_DEFINE(ENABLE_ZOOM, 1, [Zoom Support])
    AC_DEFINE(USE_ZOOM_XF86VM, 1, [Use xf86vm for zooming (deprecated)]),
    enable_zoom=no
    AC_MSG_WARN([Zoom support was requested but not found]) )
fi
if test "x$enable_zoom" = "xxrandr"; then
  if test "x$have_randr_13" = "xy"; then
    AC_DEFINE(ENABLE_ZOOM, 1, [Zoom Support])
  else
    enable_zoom=no
    AC_MSG_WARN([Zoom support was requested but not found])
  fi
fi
AM_CONDITIONAL(ENABLE_ZOOM, test "x$enable_zoom" != "xno")

AC_ARG_ENABLE(dbus,
  AS_HELP_STRING([--enable-dbus], [compile with D-Bus support (experimental) @<:@default=no@:>@]),,
  enable_dbus=no)
if test "x$enable_dbus" = "xyes"; then
  PKG_CHECK_MODULES(DBUS, dbus-1, AC_DEFINE(USE_DBUS, 1, [dbus support]), enable_dbus=no)
fi
AM_CONDITIONAL(ENABLE_DBUS, test "x$enable_dbus" = "xyes")

AC_CHECK_LIB(Fridge,mass_quantities_of_bass_ale)
AC_CHECK_LIB(Fridge,mass_quantities_of_any_ale,, [
  echo "Warning: No ales were found in your refrigerator."
  echo "  We highly suggest that you rectify this situation immediately."
])

# GNOME session support
AC_ARG_WITH(gnome,
  AS_HELP_STRING([--with-gnome], [compile with gnome support @<:@default=auto@:>@]),,
  with_gnome=auto)
if test "x$with_gnome" = "xauto"; then
  with_gnome2=yes
  with_gnome3=yes
fi
if test "x$with_gnome" = "xgnome2"; then
  with_gnome2=yes
fi
if test "x$with_gnome" = "xgnome3"; then
  with_gnome3=yes
fi
AM_CONDITIONAL(WITH_GNOME2, test "x$with_gnome2" = "xyes")
AM_CONDITIONAL(WITH_GNOME3, test "x$with_gnome3" = "xyes")

# Restore CPPFLAGS/LDFLAGS
CPPFLAGS="$SAVE_CPPFLAGS"
LDFLAGS="$SAVE_LDFLAGS"

AC_ARG_ENABLE(libhack,
  AS_HELP_STRING([--enable-libhack], [build window mode helper library @<:@default=yes@:>@]),,
  enable_libhack=yes)
if test "x$enable_libhack" = "xyes"; then
  AC_CHECK_LIB(dl, dlopen, DLOPEN_LIBS=-ldl, enable_libhack=no)
  AC_SUBST(DLOPEN_LIBS)
fi
if test "x$enable_libhack" = "xyes"; then
  AC_DEFINE(USE_LIBHACK, 1, [Use window mode helper library])
fi
AM_CONDITIONAL(BUILD_LIBHACK, test "x$enable_libhack" = "xyes")

AC_ARG_ENABLE(libtrip,
  AS_HELP_STRING([--enable-libtrip], [build debug library @<:@default=no@:>@]),,
  enable_libtrip=no)
AC_CHECK_HEADERS(execinfo.h,, enable_libtrip=no)
AC_CHECK_FUNCS(backtrace backtrace_symbols,, enable_libtrip=no)
AM_CONDITIONAL(BUILD_LIBTRIP, test "x$enable_libtrip" = "xyes")

AC_ARG_ENABLE(modules,
  AS_HELP_STRING([--enable-modules], [enable support for loadable modules @<:@default=no@:>@]),,
  enable_modules=no)
if test "x$enable_modules" = "xyes"; then
  AC_CHECK_LIB(dl, dlopen, DLOPEN_LIBS=-ldl, enable_modules=no)
  AC_SUBST(DLOPEN_LIBS)
fi
if test "x$enable_modules" = "xyes"; then
  AC_DEFINE(USE_MODULES, 1, [Loadable module support])
fi
AM_CONDITIONAL(BUILD_MODULES, test "x$enable_modules" = "xyes")

AC_ARG_ENABLE(dialogs,
  AS_HELP_STRING([--enable-dialogs],
                 [enable support for configuration dialogs @<:@default=yes@:>@]),,
  enable_dialogs=yes)
if test "x$enable_dialogs" = "xyes"; then
  AC_DEFINE(ENABLE_DIALOGS, 1, [Configuration dialog support])
fi

AC_ARG_ENABLE(containerless,
  AS_HELP_STRING([--enable-containerless],
                 [do not use container window (experimental) @<:@default=no@:>@]),,
  enable_containerless=no)
if test "x$enable_containerless" != "xyes"; then
  AC_DEFINE(USE_CONTAINER_WIN, 1, [Use container window])
fi

AM_MISSING_PROG(ASCIIDOC, asciidoc)
AC_ARG_ENABLE(mans,
  AS_HELP_STRING([--enable-mans], [install man page @<:@default=yes@:>@]),,
  enable_mans=yes)
AM_CONDITIONAL([INSTALL_MANS], [test "x$enable_mans" = "xyes"])
AC_ARG_ENABLE(docs,
  AS_HELP_STRING([--enable-docs], [install html docs @<:@default=yes@:>@]),,
  enable_docs=yes)
AM_CONDITIONAL([INSTALL_DOCS], [test "x$enable_docs" = "xyes"])

AC_ARG_ENABLE(gcc-cpp,
  AS_HELP_STRING([--enable-gcc-cpp], [compile everything as if it were C++ @<:@default=no@:>@]),,
  enable_gcc_cpp=no)
if test "x$enable_gcc_cpp" = "xyes"; then
  CC="g++"
  CPPFLAGS="$CPPFLAGS -x c++"
  EC_C_WARNINGS([cpp])
else
  EC_C_WARNINGS()
fi

EC_C_ASAN()

AC_DEFINE(USE_EXT_INIT_WIN,          1, [Use external init window during restart])

AC_DEFINE(ENABLE_DESKRAY,            0, [Enable deskrays (not functional)])

AC_SUBST(E_X_LIBS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
src/Makefile
dox/Makefile
dox/E-docs/Makefile
eesh/Makefile
epp/Makefile
lib/Makefile
config/Makefile
icons/Makefile
pix/Makefile
themes/Makefile
fonts/Makefile
scripts/Makefile
misc/Makefile
docs/Makefile
po/Makefile.in
])
AC_OUTPUT

if test "x$have_good_imlib2" != "xyes"; then
  echo "***************************************************************"
  echo "*** WARNING: imlib2 >= 1.2.0 was not detected               ***"
  echo "*** WARNING: If you indeed do not have this, expect trouble ***"
  echo "***************************************************************"
fi

echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE Version $VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration summary"
echo
echo "Support for"
echo "  Localisation ................. $USE_NLS"
echo "  Sound ........................ $enable_sound"
if test "x$enable_sound" = "xyes"; then
echo "    Default .................... $default_sound_engine"
echo "    pulseaudio ................. $enable_sound_pulse"
echo "    esound ..................... $enable_sound_esound"
echo "    alsa ....................... $enable_sound_alsa"
echo "    sndio ...................... $enable_sound_sndio"
echo "    player ..................... $enable_sound_player ($with_sound_player)"
echo "  Sound loader ................. $with_sndldr"
fi
echo "  Session management ........... $enable_sm"
echo "  Zoom ......................... $enable_zoom"
echo "  Pango ........................ $enable_pango"
echo "  Xft .......................... $enable_xft"
echo "  Xinerama ..................... $enable_xinerama"
echo "  RandR ........................ $enable_xrandr"
echo "  Render ....................... $enable_xrender"
echo "  Sync ......................... $enable_xsync"
echo "  Composite .................... $enable_composite"
echo "  GNOME session support ........ $with_gnome"
echo "  Window mode helper library ... $enable_libhack"
echo "  Dialogs ...................... $enable_dialogs"
echo
echo "Miscellaneous options"
echo "  Event handler API ............ $with_evhan"
echo "  Use Modules .................. $enable_modules"
echo "  Visibility hiding ............ $enable_visibility_hiding (only useful with modules)"
echo
echo "Experimental options - DO NOT USE unless you know what you are doing"
echo "  GLX .......................... $enable_glx"
echo "  ScreenSaver .................. $enable_xscrnsaver"
echo "  D-Bus ........................ $enable_dbus"
echo "  XI2 .......................... $enable_xi2"
echo "  Present....................... $enable_xpresent"
echo "  Do not use container window .. $enable_containerless"
echo
echo "Installation path .............. $prefix"
echo "  Install HTML docs ............ $enable_docs"
echo "  Install man page ............. $enable_mans"
echo
echo "Compilation .................... make"
echo "Installation ................... make install"
echo
