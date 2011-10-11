
bin_PROGRAMS = timeout fd fd_timeout

common_libs =  $(top_builddir)/src/libela.la
common_cflags =  -I$(top_srcdir)/include

timeout_SOURCES = timeout.c
timeout_LDADD = $(common_libs)
timeout_CFLAGS = $(common_cflags)

fd_SOURCES = fd.c
fd_LDADD = $(common_libs)
fd_CFLAGS = $(common_cflags)

fd_timeout_SOURCES = fd_timeout.c
fd_timeout_LDADD = $(common_libs)
fd_timeout_CFLAGS = $(common_cflags)
