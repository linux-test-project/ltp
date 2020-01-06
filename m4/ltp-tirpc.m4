dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_TIRPC],[
	TIRPC_CPPFLAGS=""
	TIRPC_LIBS=""

	AC_CHECK_HEADERS([tirpc/netconfig.h netconfig.h], [
		if test -n "$SDKTARGETSYSROOT"; then
			TIRPC_CPPFLAGS="-I${SDKTARGETSYSROOT}/usr/include/tirpc"
		else
			TIRPC_CPPFLAGS="-I${SYSROOT}/usr/include/tirpc"
		fi
		AC_DEFINE(HAVE_LIBTIRPC, 1, [Define to 1 if you have libtirpc headers installed])
		AC_CHECK_LIB(tirpc, rpcb_set, [TIRPC_LIBS="-ltirpc"])])

	AC_SUBST(TIRPC_CPPFLAGS)
	AC_SUBST(TIRPC_LIBS)
])
