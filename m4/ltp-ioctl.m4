dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 SUSE LLC
dnl Author: Christian Amann <camann@suse.com>

AC_DEFUN([LTP_CHECK_FIDEDUPE],[
AC_CHECK_TYPES([struct file_dedupe_range],,,[#include <linux/fs.h>])
])
