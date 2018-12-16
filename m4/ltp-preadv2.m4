dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
dnl Author: Xiao Yang <yangx.jy@cn.fujitsu.com>

dnl LTP_CHECK_PREADV2
dnl ----------------------------
AC_DEFUN([LTP_CHECK_PREADV2],[
AC_CHECK_FUNCS(preadv2,,)
])
