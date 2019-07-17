dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2014

AC_DEFUN([LTP_CHECK_CLONE_SUPPORTS_7_ARGS],[
AH_TEMPLATE(CLONE_SUPPORTS_7_ARGS,
[Define to 1 if clone() supports 7 arguments.])
AC_MSG_CHECKING([if clone() supports 7 args])
AC_TRY_LINK([#define _GNU_SOURCE
		#include <sched.h>
		#include <stdlib.h>],
		[
		#ifndef __ia64__
		clone(NULL, NULL, 0, NULL, NULL, NULL, NULL);
		#endif
		],
		AC_DEFINE(CLONE_SUPPORTS_7_ARGS) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
])
