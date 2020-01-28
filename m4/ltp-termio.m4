dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <petr.vorel@gmail.com>

AC_DEFUN([LTP_CHECK_TERMIO],[
AC_CHECK_TYPES([struct termio],,,[#include <sys/ioctl.h>])
])
