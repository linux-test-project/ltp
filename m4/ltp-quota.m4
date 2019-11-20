dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Fujitsu Ltd.
dnl Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>

AC_DEFUN([LTP_CHECK_SYSCALL_QUOTACTL],[
AC_CHECK_TYPES([struct if_nextdqblk],,,[#include <linux/quota.h>])
])
