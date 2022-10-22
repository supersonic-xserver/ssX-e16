AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = aclocal.m4 config.* compile configure depcomp \
		       install-sh ltmain.sh missing mkinstalldirs \
		       Makefile.in */Makefile.in */*/Makefile.in \
		       ABOUT-NLS \
		       po/Makefile.in.in po/Makevars.template po/Rules-quot \
		       po/*.header po/*.sed po/*.sin po/e16.pot \
		       *~ */*~

CLEANFILES = $(PACKAGE).spec version.h

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

version.h: FORCE
	@if [ -f $(top_srcdir)/.git-src -a -z "$(release)" ]; then \
	  echo '#define E16_VERSION "'`git describe --dirty`'"' > $@.tmp; \
	else \
	  echo '#define E16_VERSION "@VERSION@"' > $@.tmp; \
	fi
	@if cmp -s $@.tmp $@; then rm $@.tmp; else mv $@.tmp $@; fi

dist-hook: $(top_builddir)/$(PACKAGE).spec

RPR_DEV = `echo -n "1.%(date '+%y%m%d').git"; git rev-parse --short=8 HEAD`
RPR_REL = 1
SED_DEV = "s/\@PACKAGE\@/@PACKAGE@/;s/\@VERSION\@/@VERSION@/;s/\@RPM_RELEASE\@/$(RPR_DEV)/"
SED_REL = "s/\@PACKAGE\@/@PACKAGE@/;s/\@VERSION\@/@VERSION@/;s/\@RPM_RELEASE\@/$(RPR_REL)/"

$(top_builddir)/$(PACKAGE).spec: FORCE
	@if [ -f $(top_srcdir)/.git-src -a -z "$(release)" ]; then \
	  sed $(SED_DEV) $(top_srcdir)/$(PACKAGE).spec.in > $@; \
	else \
	  sed $(SED_REL) $(top_srcdir)/$(PACKAGE).spec.in > $@; \
	fi

.PHONY: release
release:
	$(MAKE) dist release=y

FORCE:
