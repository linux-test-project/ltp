dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Linaro Limited. All rights reserved.

dnl
dnl LTP_CHECK_SYNCFS
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYNCFS],[
AC_CHECK_FUNCS(syncfs,,)
])
