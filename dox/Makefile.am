SUBDIRS = E-docs

EXTRA_DIST = README

bin_PROGRAMS = edox

AM_CPPFLAGS = -I$(top_builddir) $(IMLIB2_CFLAGS) $(X_CFLAGS) \
	-D ENLIGHTENMENT_ROOT=\"$(pkgdatadir)\"

AM_CFLAGS = $(CFLAGS_WARNINGS) $(CFLAGS_VISIBILITY) $(CFLAGS_ASAN)

edox_SOURCES = dox.c dox.h file.c format.c text.c ttfont.c

edox_LDADD = $(IMLIB2_LIBS) $(X_LIBS) $(XINERAMA_LIBS) -lX11 -lm
