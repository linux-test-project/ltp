dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>

AC_DEFUN([LTP_CHECK_MMSGHDR],[
AC_CHECK_TYPES([struct mmsghdr],,,[
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
])
])
