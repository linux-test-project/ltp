dnl
dnl Copyright (c) Copyrights-are-for-losers, Inc 2010
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
dnl LTP_CHECK_TIME
dnl
dnl For all directly related time syscalls.
dnl
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_TIME],[
	AC_CHECK_DECLS([CLOCK_MONOTONIC_RAW, CLOCK_REALTIME_COARSE, CLOCK_MONOTONIC_COARSE],,,[
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <time.h>
])
])
