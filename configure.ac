#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([libela], [0.3], [sdk@freebox.fr])
AC_CONFIG_MACRO_DIR([autotools])
AC_CONFIG_AUX_DIR([autotools])

AC_CONFIG_SRCDIR([src/ela.c])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.10 foreign dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC

if test "x$GCC" = "xyes"; then
	GCC_CFLAGS="-Wall -g -Wstrict-prototypes -Wmissing-prototypes -fvisibility=hidden"
fi
AC_SUBST(GCC_CFLAGS)

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h])

AC_PATH_PROG([mkdoc], [mkdoc])
AM_CONDITIONAL(HAVE_MKDOC, test x$mkdoc != x)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_UINT32_T

AM_PROG_LIBTOOL

# Checks for libraries.

AC_CHECK_LIB(rt, clock_gettime, [
             have_librt="yes"
             LIBRT_LIBS="-lrt"
             ])
AC_SUBST(LIBRT_LIBS)

AC_ARG_WITH([libevent],
            [AS_HELP_STRING([--with-libevent],
              [Build with libevent support])],
            [],
            [with_libevent=check])

AM_CONDITIONAL(HAVE_LIBEVENT, false)
PKG_CHECK_MODULES(LIBEVENT, libevent,
                  [AC_DEFINE([HAVE_LIBEVENT], [1], [Has libevent])
                   AM_CONDITIONAL(HAVE_LIBEVENT, true)],
                  [AS_IF([test "x$with_libevent" = xyes], AC_ERROR(No libevent support))])

AX_CHECK_LINK_FLAG([-framework CoreFoundation],
                   [target_is_apple=1],
                   [target_is_apple=0])

if test "x$target_is_apple" = "x1" ; then
    AC_DEFINE(HAVE_CORE_FOUNDATION, [1], [Have core foundation available])
fi

AM_CONDITIONAL(HAVE_CORE_FOUNDATION, test "x$target_is_apple" = "x1")


AC_CONFIG_FILES([
    ela.pc
    Makefile
    include/Makefile
    include/ela/Makefile
    doc/Makefile
    src/Makefile
    test/Makefile
    ])
AC_OUTPUT
