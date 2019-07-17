dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems Inc., 2009
dnl Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

AC_DEFUN([LTP_CHECK_SYSCALL_NUMA], [
	AC_CHECK_LIB(numa, numa_available, [have_libnuma=yes])
	AC_CHECK_HEADERS([numa.h numaif.h], [], [have_numa_headers=no])

	if test "x$have_numa_headers" != "xno"; then
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <numa.h>
		], [
#if LIBNUMA_API_VERSION < 2
# error Required numa headers >= 2
#endif
		])], [have_numa_headers_v2=yes])
	fi

	if test "x$have_libnuma" = "xyes" -a "x$have_numa_headers" != "xno" -a "x$have_numa_headers_v2" = "xyes"; then
		AC_SUBST(NUMA_LIBS, "-lnuma")
		AC_DEFINE(HAVE_NUMA_V2, 1, [Define to 1 if you have libnuma and it's headers version >= 2 installed.])
	fi
])
