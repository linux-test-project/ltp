dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>

AC_DEFUN([LTP_CHECK_LIBMNL], [
    PKG_CHECK_MODULES([LIBMNL], [libmnl], [
        AC_DEFINE([HAVE_LIBMNL], [1], [Define to 1 if you have libmnl library and headers])
	], [have_libmnl=no])
])
