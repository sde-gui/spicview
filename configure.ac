dnl Process this file with autoconf to produce a configure script.

AC_INIT(spicview, 0.2.4-alpha)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_HEADER([config.h])

AC_LANG_C
AM_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_C_O
AC_HEADER_STDC

AC_ARG_ENABLE(more_warnings,
       [AC_HELP_STRING([--enable-more-warnings],
               [Add more warnings @<:@default=no@:>@])],
       [enable_more_warnings="${enableval}"],
       [enable_more_warnings=no]
)

if test x"$enable_more_warnings" = x"yes"; then
  ADDITIONAL_FLAGS="-Wall -Werror=all -Werror=format -Werror=implicit-function-declaration -Werror=implicit-int -Werror=missing-braces -Werror=parentheses -Werror=return-type -Werror=strict-aliasing -Werror=switch -Wuninitialized -Werror=unused-label -Werror=unused-value -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Werror=missing-declarations -Wredundant-decls -Wmissing-noreturn -Wpointer-arith -Wcast-align -Wwrite-strings -Werror=inline -Werror=format-nonliteral -Wformat-nonliteral -Werror=format-security -Wformat-security -Winit-self -Werror=missing-include-dirs -Werror=undef -Werror=aggregate-return -Wmissing-format-attribute -Werror=nested-externs -fno-strict-aliasing -fmessage-length=0 -Wp,-D_FORTIFY_SOURCE=2 -DG_DISABLE_DEPRECATED -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES"
fi
AC_SUBST(ADDITIONAL_FLAGS)


AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.40.0])

gtk_modules="gtk+-2.0 >= 2.24.0"
PKG_CHECK_MODULES(GTK, [$gtk_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

pkg_modules="x11"

PKG_CHECK_MODULES(X11, [$pkg_modules])
AC_CHECK_LIB(jpeg, jpeg_start_decompress, [],
	     [AC_MSG_ERROR([libjpeg not found])], -lm)
AC_CHECK_HEADER(jpeglib.h, , [AC_MSG_ERROR([libjpeg not found])])
LIBJPEG='-ljpeg'

AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

pkg_modules="libsmfm-gtk2 libsmfm-core"
PKG_CHECK_MODULES(LIBFM, [$pkg_modules])
AC_SUBST(LIBFM_CFLAGS)
AC_SUBST(LIBFM_LIBS)

GETTEXT_PACKAGE=spicview
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
po/Makefile.in
])

echo
echo "$PACKAGE_NAME $VERSION"
echo
echo "Prefix: $prefix"
echo
echo "X11_CFLAGS:   $X11_CFLAGS"
echo
echo "X11_LIBS:     $X11_LIBS"
echo
echo "GTK_CFLAGS:   $GTK_CFLAGS"
echo
echo "GTK_LIBS:     $GTK_LIBS"
echo
echo "LIBFM_CFLAGS: $LIBFM_CFLAGS"
echo
echo "LIBFM_LIBS:   $LIBFM_LIBS"
echo

