e16docdir = ${datadir}/doc/${PACKAGE}

EXTRA_DIST = e16.asc e16.man.in e16.html

CLEANFILES = e16.1
MAINTAINERCLEANFILES = e16.html

MANSUBST = \
	-e "s=\@VERSION\@=${VERSION}=" \
	-e "s=\@DOCDIR\@=$(e16docdir)="

e16.1: e16.man.in
	sed $(MANSUBST) $< > $@

 AD_OPTIONS = -a toc -a icons

e16.html: e16.asc
	$(ASCIIDOC) $(AD_OPTIONS) -o $@ $<

if INSTALL_MANS

man_MANS = e16.1

endif

if INSTALL_DOCS

dist_e16doc_DATA = e16.html

endif
