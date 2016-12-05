## Process this file with automake to produce Makefile.in

SUBDIRS = src po

EXTRA_DIST = \
	autogen.sh \
	spicview.desktop.in

desktopdir = $(datadir)/applications
desktop_in_files = spicview.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = gresource/icons/spicview.png

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi


cppcheck cppcheck-inconclusive cppcheck-check-config:
	make -C src $@
