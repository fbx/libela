
lib_LTLIBRARIES = libela.la

libela_la_SOURCES = ela.c
libela_la_CPPFLAGS = -I$(top_srcdir)/include -I.
libela_la_CFLAGS = $(GCC_CFLAGS)
libela_la_LIBADD = $(LIBRT_LIBS)
libela_la_LDFLAGS =

if HAVE_LIBEVENT
libela_la_SOURCES += ela_libevent.c
libela_la_CPPFLAGS += $(LIBEVENT_CFLAGS)
libela_la_LIBADD += $(LIBEVENT_LIBS)
endif

if HAVE_CORE_FOUNDATION
libela_la_SOURCES += ela_cf.c
libela_la_CPPFLAGS += -framework CoreFoundation
libela_la_LDFLAGS += -framework CoreFoundation
endif
