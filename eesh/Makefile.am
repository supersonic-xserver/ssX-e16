
bin_PROGRAMS = eesh

eesh_SOURCES = E.h comms.c main.c

eesh_CPPFLAGS = -I$(top_builddir) $(X_CFLAGS) $(CFLAGS_WARNINGS)

eesh_CFLAGS = $(CFLAGS_ASAN)

LDADD = $(X_LIBS) -lX11

