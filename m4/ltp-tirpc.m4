dnl
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it would be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write the Free Software Foundation,
dnl Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
dnl

dnl
dnl LTP_CHECK_TIRPC
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_TIRPC],[
	TIRPC_CPPFLAGS=""
	TIRPC_LIBS=""

	AC_CHECK_HEADER(tirpc/netconfig.h,[
		TIRPC_CPPFLAGS="-I${SYSROOT}/usr/include/tirpc"
		AC_DEFINE(HAVE_LIBTIRPC, 1, [Define to 1 if you have libtirpc headers installed])
		AC_CHECK_LIB(tirpc, rpcb_set, [TIRPC_LIBS="-ltirpc"])])

	AC_SUBST(TIRPC_CPPFLAGS)
	AC_SUBST(TIRPC_LIBS)
])
