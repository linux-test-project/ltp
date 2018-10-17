dnl
dnl Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
dnl
dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl

dnl
dnl LTP_CHECK_PERF_EVENT
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_PERF_EVENT],[
AC_CHECK_MEMBERS([struct perf_event_mmap_page.aux_head],,,[#include <linux/perf_event.h>])
])
