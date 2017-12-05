dnl
dnl Copyright (c) Cisco Systems Inc., 2009
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
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>
dnl Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
dnl

dnl
dnl LTP_CHECK_SYSCALL_NUMA
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_NUMA], [
	AC_CHECK_LIB(numa, numa_available, [have_libnuma=yes])
	AC_CHECK_HEADERS([numa.h numaif.h], [], [have_numa_headers=no])

	if test "x$have_numa_headers" != "xno"; then
		AC_RUN_IFELSE([AC_LANG_PROGRAM([
#include <numa.h>
		], [
#if LIBNUMA_API_VERSION < 2
exit(1);
#endif
		])], [have_numa_headers_v2=yes])
	fi

	if test "x$have_libnuma" = "xyes" -a "x$have_numa_headers" != "xno" -a "x$have_numa_headers_v2" = "xyes"; then
		AC_SUBST(NUMA_LIBS, "-lnuma")
		AC_DEFINE(HAVE_NUMA_V2, 1, [Define to 1 if you have libnuma and it's headers version >= 2 installed.])
	fi
])
