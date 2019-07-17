dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_SYSCALL_UTIMENSAT],[
	AC_MSG_CHECKING([for utimensat])
	AC_LINK_IFELSE([AC_LANG_SOURCE([
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
	long tv_nsec;
	tv_nsec = UTIME_NOW;
	tv_nsec = UTIME_OMIT;

	return utimensat(AT_FDCWD, NULL, NULL, 0);
}])],[has_utimensat="yes"])

if test "x$has_utimensat" = "xyes"; then
	AC_DEFINE(HAVE_UTIMENSAT, 1, [Define to 1 if you have utimensat(2)])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
