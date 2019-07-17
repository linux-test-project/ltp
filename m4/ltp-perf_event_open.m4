dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2014 Fujitsu Ltd.
dnl Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
dnl Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>

AC_DEFUN([LTP_CHECK_SYSCALL_PERF_EVENT_OPEN],[
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
