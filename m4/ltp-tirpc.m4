dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_TIRPC], [
	dnl libtirpc library and headers
	PKG_CHECK_MODULES([LIBTIRPC], [libtirpc >= 0.2.4], [
		have_libtirpc=yes
		TIRPC_CFLAGS=$LIBTIRPC_CFLAGS
		TIRPC_LIBS=$LIBTIRPC_LIBS
	], [have_libtirpc=no])

	dnl TI-RPC headers (in glibc, since 2.26 installed only when configured
	dnl with --enable-obsolete-rpc)
	dnl NOTE: To port tests for ntirpc would require use non-deprecated
	dnl functions as it does not have the deprecated ones any more (e.g. use
	dnl rpc_broadcast() instead of clnt_broadcast()), but glibc implementation
	dnl does not have the new ones. We could either provide the deprecated
	dnl functions (copy from libtirpc src/rpc_soc.c) or drop glibc tests.
	AC_CHECK_FUNCS([xdr_char clnttcp_create], [have_rpc_glibc=yes])

	if test "x$have_libtirpc" = "xyes" -o "x$have_rpc_glibc" = "xyes"; then
		AC_SUBST(HAVE_RPC, 1)
	fi

	dnl fix for old pkg-config (< 0.24)
	dnl https://autotools.io/pkgconfig/pkg_check_modules.html
	AC_SUBST(TIRPC_CFLAGS)
	AC_SUBST(TIRPC_LIBS)
])
