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
dnl

dnl
dnl LTP_CHECK_SYSCALL_NUMA
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_NUMA],
[dnl
AC_CHECK_HEADERS([linux/mempolicy.h numa.h numaif.h],[
	LTP_SYSCALL_NUMA_HEADERS=yes
	AC_CHECK_LIB(numa,numa_alloc_onnode,[have_numa_alloc_onnode="yes"])
	if  test "x$have_numa_alloc_onnode" = "xyes"; then
		AC_DEFINE(HAVE_NUMA_ALLOC_ONNODE,1,[define to 1 if you have 'numa_alloc_onnode' function])
	fi
	AC_CHECK_LIB(numa,numa_move_pages,[have_numa_move_pages="yes"])
	if  test "x$have_numa_move_pages" = "xyes"; then
		AC_DEFINE(HAVE_NUMA_MOVE_PAGES,1,[define to 1 if you have 'numa_move_pages' function])
	fi
]
	AC_CHECK_LIB(numa,numa_available,[
NUMA_CPPFLAGS="-DNUMA_VERSION1_COMPATIBILITY"
NUMA_LIBS="-lnuma"
	])
dnl For testcases/kernel/controllers/cpuset, testcases/kernel/syscalls/get_mempolicy,
dnl testcases/kernel/syscalls/mbind
AC_CHECK_DECLS([MPOL_BIND, MPOL_DEFAULT, MPOL_F_ADDR, MPOL_F_MEMS_ALLOWED, MPOL_F_NODE, MPOL_INTERLEAVE, MPOL_PREFERRED],[have_mpol_constants="yes"],,[#include <numaif.h>])
AC_SUBST(NUMA_CPPFLAGS)
AC_SUBST(NUMA_LIBS)
if test "x$have_mpol_constants" = "xyes"; then
	AC_DEFINE(HAVE_MPOL_CONSTANTS,1,[define to 1 if you have all constants required to use mbind tests])
fi
)])
