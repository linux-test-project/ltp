dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 SUSE LLC
dnl Author: Christian Amann <camann@suse.com>

AC_DEFUN([LTP_CHECK_ACCT],[
AC_CHECK_TYPES([struct acct_v3],,,[#include <sys/acct.h>])
])
