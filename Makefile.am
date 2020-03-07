AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = aclocal.m4 config.* compile configure depcomp \
		       install-sh ltmain.sh missing mkinstalldirs \
		       Makefile.in */Makefile.in */*/Makefile.in \
		       ABOUT-NLS \
		       po/Makefile.in.in po/Makevars.template po/Rules-quot \
		       po/*.header po/*.sed po/*.sin po/e16.pot \
		       *~ */*~

CLEANFILES = $(PACKAGE).spec

SUBDIRS = dox eesh epp src lib config themes fonts scripts misc docs icons pix po

EXTRA_DIST = \
$(PACKAGE).spec.in \
sample-scripts/bouncingball.pl \
sample-scripts/lcdmover.sh \
sample-scripts/testroller.pl \
sample-scripts/shade-pagers.pl \
sample-scripts/place-pagers.pl \
AUTHORS COMPLIANCE COPYING ChangeLog

if SRC_IS_GIT
RPM_RELEASE=`echo -n "1.%(date '+%y%m%d').git"; git rev-parse --short=8 HEAD`
else
RPM_RELEASE=1
endif

dist-hook: $(top_builddir)/$(PACKAGE).spec

$(top_builddir)/$(PACKAGE).spec: FORCE
	sed "s/\@PACKAGE\@/@PACKAGE@/;s/\@VERSION\@/@VERSION@/;s/\@RPM_RELEASE\@/$(RPM_RELEASE)/" \
		$(top_srcdir)/$(PACKAGE).spec.in > $@

FORCE:
