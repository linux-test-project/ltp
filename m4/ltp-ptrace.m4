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
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl


AC_DEFUN([LTP_CHECK_LINUX_PTRACE],
_LTP_CHECK_LINUX_PTRACE
)dnl

dnl _LTP_CHECK_LINUX_PTRACE
dnl ----------------------------------------
dnl
dnl Check for ptrace support
dnl in commit 016ae219 in July 2008
dnl

AC_DEFUN([_LTP_CHECK_LINUX_PTRACE],[
AC_CHECK_HEADERS(linux/ptrace.h,[
	LINUX_PTRACE_HEADER=yes
	AC_CHECK_DECLS([PTRACE_GETSIGINFO, PTRACE_O_TRACEVFORKDONE, PTRACE_SETOPTIONS],[],[],[dnl
#include <linux/ptrace.h>
]) dnl AC_CHECK_DECLS
]) dnl AC_CHECK_HEADERS
]) dnl _LTP_CHECK_TASKSTATS_FREEPAGES
