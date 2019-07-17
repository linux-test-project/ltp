dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2014 Linux Test Project

AC_DEFUN([LTP_CHECK_IOVEC],[
AC_CHECK_TYPES([struct iovec],,,[#include <sys/uio.h>])
])
