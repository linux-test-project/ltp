dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2015
dnl Author: Cedric Hnyda <chnyda@suse.com>

AC_DEFUN([LTP_CHECK_KCMP_TYPE],[
AC_CHECK_TYPES([enum kcmp_type],,,[#include <linux/kcmp.h>])
])
