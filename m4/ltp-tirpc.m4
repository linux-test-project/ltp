dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_TIRPC], [
	dnl libtirpc library and headers
	PKG_CHECK_MODULES([LIBTIRPC], [libtirpc], [
		TIRPC_CFLAGS=$LIBTIRPC_CFLAGS
		TIRPC_LIBS=$LIBTIRPC_LIBS
	], [have_libtirpc=no])

	dnl fix for old pkg-config (< 0.24)
	dnl https://autotools.io/pkgconfig/pkg_check_modules.html
	AC_SUBST(TIRPC_CFLAGS)
	AC_SUBST(TIRPC_LIBS)
])
