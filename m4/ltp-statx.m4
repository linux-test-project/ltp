dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Zilogic Systems Pvt. Ltd., 2018

AC_DEFUN([LTP_CHECK_STATX],[
AC_CHECK_FUNCS(statx,,)
AC_CHECK_HEADER(linux/fs.h,,,)
AC_CHECK_TYPES([struct statx],,,[[
	#define _GNU_SOURCE
	#include <sys/stat.h>
]])
AC_CHECK_TYPES([struct statx_timestamp],,,[[
	#define _GNU_SOURCE
	#include <sys/stat.h>]])
])
