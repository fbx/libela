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

#AM_SILENT_RULES([yes])

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

LIBEVENT_LIBS=""
if test x"$with_libevent" = x"no" ; then
   have_libevent=no
else
   AC_CHECK_HEADERS(event.h, [
                    have_event_h="yes"
                    ])
   AC_CHECK_LIB(event, event_set, [
                have_libevent="$have_event_h"
                LIBEVENT_LIBS="-levent"
                ])
fi

if test "x$have_libevent" = "xyes" ; then
   AC_DEFINE(HAVE_EVENT_H)
   AC_SUBST(LIBEVENT_LIBS)
fi
AM_CONDITIONAL(HAVE_LIBEVENT, test "x$have_libevent" = "xyes")

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
