## Process this file with automake to produce Makefile.in

# set the include path found by configure
INCLUDES = $(all_includes) \
        $(GTK_CFLAGS) \
        $(ADDITIONAL_FLAGS) \
        $(LIBSMFM_CFLAGS) \
        $(GMODULE_CFLAGS) \
	-I$(top_srcdir) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_NAME_STR=\""$(PACKAGE_NAME)"\"

bin_PROGRAMS = spicview
spicview_SOURCES = \
	spicview.c \
	image-list.c \
	main-win.c \
	image-view.c \
	ptk-menu.c \
	file-dlgs.c \
	jpeg-tran.c \
	tran-supp.c \
	pref.c \
	jhead.c \
	exif.c \
	jpgfile.c \
	image-cache.c \
	libsmfm_utils.c \
	info-window.c \
	spicview-gresource.c

# the library search path.
# use -nodefaultlibs to prevent linking libstdc++
spicview_LDADD = $(GTK_LIBS) $(LIBSMFM_LIBS) -lm

noinst_HEADERS = \
	gtkcompat.h \
	spicview.h \
	image-list.h \
	main-win.h \
	image-view.h \
	ptk-menu.h \
	file-dlgs.h \
	jpeg-tran.h \
	tran-supp.h \
	pref.h \
	jhead.h \
	image-cache.h \
	libsmfm_utils.h \
	info-window.h \
	spicview-gresource.h

spicview-gresource.c: spicview.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/../gresource --generate-dependencies spicview.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/../gresource --manual-register --generate-source --target=$@ $<

spicview-gresource.h: spicview.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/../gresource --generate-dependencies spicview.gresource.xml)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir)/../gresource --manual-register --generate-header --target=$@ $<

BUILT_SOURCES = spicview-gresource.c spicview-gresource.h

CLEANFILES = $(BUILT_SOURCES) 

EXTRA_DIST = \
	spicview.gresource.xml

