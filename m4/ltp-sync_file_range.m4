dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Linaro Limited. All rights reserved.

dnl
dnl LTP_CHECK_SYNC_FILE_RANGE
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYNC_FILE_RANGE],[
AC_CHECK_FUNCS(sync_file_range,,)
])
