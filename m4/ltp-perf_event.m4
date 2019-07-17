dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_PERF_EVENT],[
AC_CHECK_MEMBERS([struct perf_event_mmap_page.aux_head],,,[#include <linux/perf_event.h>])
])
