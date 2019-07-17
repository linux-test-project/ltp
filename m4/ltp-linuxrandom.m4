dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2015

AC_DEFUN([LTP_CHECK_LINUXRANDOM],[
	AC_MSG_CHECKING([for linux/random.h])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([

#include <linux/random.h>
int main(void) {
	return 0;
}])],[has_linux_random_h="yes"])

if test "x$has_linux_random_h" = xyes; then
	AC_DEFINE(HAVE_LINUX_RANDOM_H,1,[Define to 1 if having a valid linux/random.h])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
