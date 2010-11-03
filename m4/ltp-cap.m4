dnl
dnl Copyright (c) Cisco Systems Inc., 2009
dnl Copyright (c) Linux Test Project, 2010
dnl
dnl This program is free software;  you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl
dnl Author: Garrett Cooper <yanegomi@gmail.com>
dnl

dnl
dnl LTP_CHECK_CAPABILITY_SUPPORT
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_CAPABILITY_SUPPORT],[
AH_TEMPLATE(HAVE_LIBCAP,
[Define to 1 if you have libcap-2 installed.])
AC_CHECK_HEADER([sys/capability.h],[capability_header_prefix="sys"])
if test "x$capability_header_prefix" != x; then
	AC_CHECK_LIB(cap,cap_compare,[cap_libs="-lcap"])
	AC_CHECK_PROG(CAPSET,capset,capset)
fi
if test "x$cap_libs" != x; then
	AC_DEFINE(HAVE_LIBCAP)
fi
AC_SUBST(CAP_LIBS,$cap_libs)
])
