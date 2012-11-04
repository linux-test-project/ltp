dnl
dnl Copyright (c) Jiri Palecek 2009
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

AC_DEFUN([LTP_CHECK_TASKSTATS],
_LTP_CHECK_TASKSTATS_FREEPAGES
)dnl

dnl _LTP_CHECK_TASKSTATS_FREEPAGES
dnl ----------------------------------------
dnl
dnl Check for taskstat.freepages_* members, introduced to the kernel
dnl in commit 016ae219 in July 2008
dnl

AC_DEFUN([_LTP_CHECK_TASKSTATS_FREEPAGES],[
AC_CHECK_HEADERS([linux/taskstats.h],[
    AC_CHECK_MEMBERS([struct taskstats.freepages_count, struct taskstats.nvcsw, struct taskstats.read_bytes],
                     [],[],[dnl
#include <linux/taskstats.h>
]) dnl AC_CHECK_MEMBERS
],[],[
#include <linux/types.h>
]) dnl AC_CHECK_HEADERS_ONCE
]) dnl _LTP_CHECK_TASKSTATS_FREEPAGES
