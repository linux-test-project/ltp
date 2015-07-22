dnl
dnl Copyright (c) Linux Test Project, 2015
dnl
dnl This program is free software;  you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY;  without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
dnl the GNU General Public License for more details.
dnl

dnl
dnl LTP_CHECK_LINUXRANDOM
dnl ----------------------------
dnl

AC_DEFUN([LTP_CHECK_LINUXRANDOM],[dnl
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
