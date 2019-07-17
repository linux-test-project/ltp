dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

dnl For all directly related time syscalls
AC_DEFUN([LTP_CHECK_TIME],[
	AC_CHECK_DECLS([CLOCK_MONOTONIC_RAW, CLOCK_REALTIME_COARSE, CLOCK_MONOTONIC_COARSE],,,[
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <time.h>
])
])
