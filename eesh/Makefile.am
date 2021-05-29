
bin_PROGRAMS = eesh

AM_CPPFLAGS = -I$(top_builddir) $(X_CFLAGS)

AM_CFLAGS = $(CFLAGS_WARNINGS) $(CFLAGS_VISIBILITY) $(CFLAGS_ASAN)

eesh_SOURCES = E.h comms.c main.c

eesh_LDADD = $(X_LIBS) -lX11
