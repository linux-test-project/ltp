dnl
dnl Copyright (c) 2014 Fujitsu Ltd.
dnl Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
dnl Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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

dnl
dnl LTP_CHECK_SYSCALL_PERF_EVENT_OPEN
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_PERF_EVENT_OPEN],[dnl
AH_TEMPLATE(HAVE_PERF_EVENT_ATTR,
[Define to 1 if you have struct perf_event_attr])
AC_MSG_CHECKING([for perf_event_attr in linux/perf_event.h])
AC_TRY_COMPILE([#include <unistd.h>
		#include <linux/perf_event.h>],
		[
			struct perf_event_attr pe;
		],
		AC_DEFINE(HAVE_PERF_EVENT_ATTR) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
])
