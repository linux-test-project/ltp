dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2018 Google, Inc.

AC_DEFUN([LTP_CHECK_RLIMIT64],[
AC_CHECK_TYPES([struct rlimit64],,,[
#define _LARGEFILE64_SOURCE
#include <sys/resource.h>
])
])
