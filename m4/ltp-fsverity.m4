dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2022 Fujitsu Ltd.
dnl Author: Dai Shili <daisl.fnst@fujitsu.com>

AC_DEFUN([LTP_CHECK_FSVERITY],[
	AC_CHECK_HEADERS([linux/fsverity.h], [have_fsverity=yes], [AC_MSG_WARN(missing linux/fsverity.h header)])
	if test "x$have_fsverity" = "xyes"; then
		AC_CHECK_TYPES(struct fsverity_enable_arg,,,[#include <linux/fsverity.h>])
	fi
])
