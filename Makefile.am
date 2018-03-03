AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = aclocal.m4 config.* compile configure depcomp \
		       install-sh ltmain.sh missing mkinstalldirs \
		       Makefile.in */Makefile.in */*/Makefile.in \
		       version.h \
		       ABOUT-NLS \
		       po/Makefile.in.in po/Makevars.template po/Rules-quot \
		       po/*.header po/*.sed po/*.sin po/e16.pot \
		       *~ */*~

CLEANFILES = $(PACKAGE).spec

BUILT_SOURCES = version.h

SUBDIRS = dox eesh epp src lib config themes fonts scripts misc docs icons pix po

EXTRA_DIST = \
$(PACKAGE).spec.in version.h \
sample-scripts/bouncingball.pl \
sample-scripts/lcdmover.sh \
sample-scripts/testroller.pl \
sample-scripts/shade-pagers.pl \
sample-scripts/place-pagers.pl \
AUTHORS COMPLIANCE COPYING ChangeLog

if SRC_IS_GIT
version.h: FORCE
	@echo '#define E16_VERSION "'`git describe --dirty`'"' > $@.tmp
	@if cmp -s $@.tmp $@; then rm $@.tmp; else mv $@.tmp $@; fi

RPM_RELEASE=`echo -n "1.%(date '+%y%m%d').git"; git rev-parse --short=8 HEAD`
else
version.h:
	@echo '#define E16_VERSION "@VERSION@"' > $@.tmp
	@if cmp -s $@.tmp $@; then rm $@.tmp; else mv $@.tmp $@; fi

RPM_RELEASE=1
endif

dist-hook: $(top_builddir)/$(PACKAGE).spec

$(top_builddir)/$(PACKAGE).spec: FORCE
	sed "s/\@PACKAGE\@/@PACKAGE@/;s/\@VERSION\@/@VERSION@/;s/\@RPM_RELEASE\@/$(RPM_RELEASE)/" \
		$(top_srcdir)/$(PACKAGE).spec.in > $@

FORCE:
