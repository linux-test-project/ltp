dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2012
dnl Author: Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_FS_IOC_FLAGS],[
AH_TEMPLATE(HAVE_FS_IOC_FLAGS,
[Define to 1 if you have FS_IOC_GETFLAGS and FS_IOC_SETFLAGS in <linux/fs.h>.])
AC_MSG_CHECKING([for FS_IOC_GETFLAGS and FS_IOC_SETFLAGS in <linux/fs.h>])
AC_TRY_COMPILE([#include <linux/fs.h>], [int flags = FS_IOC_GETFLAGS;],
               AC_DEFINE(HAVE_FS_IOC_FLAGS) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
])
