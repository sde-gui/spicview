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
	libsmfm_utils.c

# the library search path.
# use -nodefaultlibs to prevent linking libstdc++
spicview_LDADD = $(GTK_LIBS) $(LIBSMFM_LIBS) -lm

noinst_HEADERS = \
	glib-mem.h \
	gtkcompat.h \
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
	libsmfm_utils.h

