dnl Process this file with autoconf to produce a configure script.

AC_INIT(spicview, 0.3.0-alpha)

dnl ===================

# Argument quoting is from gcc's configure

# Get original configure arguments. Quote arguments with shell meta charatcers.
CONFIGURE_ARGUMENTS=
for ac_arg in "$0" "$@"
do
  case "$ac_arg" in
  *" "*|*"	"*|*[[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?\']]*)
    ac_arg=`echo "$ac_arg" | sed "s/'/'\\\\\\\\''/g"`
    # if the argument is of the form -foo=baz, quote the baz part only
    ac_arg=`echo "'$ac_arg'" | sed "s/^'\([[-a-zA-Z0-9]]*=\)/\\1'/"` ;;
  *) ;;
  esac
  # Add the quoted argument to the list.
  CONFIGURE_ARGUMENTS="$CONFIGURE_ARGUMENTS $ac_arg"
done

# Remove the initial space we just introduced and, as these will be
# expanded by make, quote '$'.
CONFIGURE_ARGUMENTS=`echo "x$CONFIGURE_ARGUMENTS" | sed -e 's/^x *//' -e 's,\\$,$$,g'`

AC_SUBST(CONFIGURE_ARGUMENTS)
AH_TEMPLATE(CONFIGURE_ARGUMENTS, [])
AC_DEFINE_UNQUOTED(CONFIGURE_ARGUMENTS,"$CONFIGURE_ARGUMENTS", [])

dnl ===================

AC_CONFIG_AUX_DIR([build-aux])
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

ADDITIONAL_FLAGS="-Werror=implicit-function-declaration -Werror=implicit-int"

if test x"$enable_more_warnings" = x"yes"; then
  ADDITIONAL_FLAGS="$ADDITIONAL_FLAGS -Wall -Werror=all -Werror=format -Werror=missing-braces -Werror=parentheses -Werror=return-type -Werror=strict-aliasing -Werror=switch -Wuninitialized -Werror=unused-label -Werror=unused-value -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Werror=missing-declarations -Wredundant-decls -Wmissing-noreturn -Wpointer-arith -Wcast-align -Wwrite-strings -Werror=inline -Werror=format-nonliteral -Wformat-nonliteral -Werror=format-security -Wformat-security -Winit-self -Werror=missing-include-dirs -Werror=undef -Werror=aggregate-return -Wmissing-format-attribute -Werror=nested-externs -fno-strict-aliasing -fmessage-length=0 -DG_DISABLE_DEPRECATED -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES"
fi
AC_SUBST(ADDITIONAL_FLAGS)


AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.40.0])

AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

gtk_modules="gtk+-2.0 >= 2.24.0 gdk-pixbuf-2.0 >= 2.26 gio-2.0 >= 2.32"
PKG_CHECK_MODULES(GTK, [$gtk_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_CHECK_LIB(jpeg, jpeg_start_decompress, [],
	     [AC_MSG_ERROR([libjpeg not found])], -lm)
AC_CHECK_HEADER(jpeglib.h, , [AC_MSG_ERROR([libjpeg not found])])
LIBJPEG='-ljpeg'


AC_ARG_WITH(libsmfm, [  --without-libsmfm       Compile without libsmfm support])
if test "x$with_libsmfm" != "xno"; then
    pkg_modules="libsmfm-gtk2 libsmfm-core"
    PKG_CHECK_MODULES(LIBSMFM, [$pkg_modules])
    AH_TEMPLATE(ENABLE_LIBSMFM, [])
    AC_DEFINE(ENABLE_LIBSMFM)
fi
AC_SUBST(LIBSMFM_CFLAGS)
AC_SUBST(LIBSMFM_LIBS)

GETTEXT_PACKAGE=spicview
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])

echo
echo "$PACKAGE_NAME $VERSION"
echo
echo "Prefix: $prefix"
echo
echo "GTK_CFLAGS:   $GTK_CFLAGS"
echo
echo "GTK_LIBS:     $GTK_LIBS"
echo
echo "LIBSMFM_CFLAGS: $LIBSMFM_CFLAGS"
echo
echo "LIBSMFM_LIBS:   $LIBSMFM_LIBS"
echo

