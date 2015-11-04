#!/bin/sh -x

libtoolize --force
aclocal
autoheader
automake --add-missing
autoconf
