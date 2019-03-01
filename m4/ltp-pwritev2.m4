dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
dnl Author: Jinhui Huang <huangjh.jy@cn.fujitsu.com>

dnl LTP_CHECK_PWRITEV2
dnl ----------------------------
AC_DEFUN([LTP_CHECK_PWRITEV2],[
AC_CHECK_FUNCS(pwritev2,,)
])
