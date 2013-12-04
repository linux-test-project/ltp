dnl
dnl Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it would be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write the Free Software Foundation,
dnl Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
dnl

dnl
dnl LTP_CHECK_SYSCALL_UTIMENSAT
dnl ----------------------------
dnl
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
