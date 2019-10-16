dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems Inc., 2009
dnl Copyright (c) Linux Test Project, 2019
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

AC_DEFUN([LTP_CHECK_CAPABILITY_SUPPORT],[
AH_TEMPLATE(HAVE_LIBCAP,
[Define to 1 if you have libcap-2 installed.])
AC_CHECK_HEADERS([sys/capability.h],[capability_header_prefix="sys"])
if test "x$capability_header_prefix" != x; then
	AC_CHECK_LIB(cap,cap_compare,[cap_libs="-lcap"])
fi
if test "x$cap_libs" != x; then
	AC_DEFINE(HAVE_LIBCAP)
fi
AC_SUBST(CAP_LIBS,$cap_libs)

AH_TEMPLATE(HAVE_NEWER_LIBCAP,
[Define to 1 if you have newer libcap-2 installed.])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <sys/capability.h>
#include <linux/types.h>
int main(void) {
	__u16 a;
	__u32 b;
	return 0;
}])],[has_newer_libcap="yes"])

if test "x$has_newer_libcap" = xyes; then
	AC_DEFINE(HAVE_NEWER_LIBCAP)
fi
])
