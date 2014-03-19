dnl
dnl Copyright (c) 2014 Fujitsu Ltd.
dnl Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
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
dnl You should have received a copy of the GNU General Public License
dnl along with this program;  if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
dnl

dnl
dnl LTP_CHECK_SYSCALL_FCNTL
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_FCNTL],[dnl
	AC_MSG_CHECKING([for fcntl f_owner_ex])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#define _GNU_SOURCE
#include <fcntl.h>
int main(void) {
	struct f_owner_ex tst_own_ex;
	return 0;
}])],[has_f_owner_ex="yes"])

if test "x$has_f_owner_ex" = xyes; then
	AC_DEFINE(HAVE_STRUCT_F_OWNER_EX,1,[Define to 1 if you have struct f_owner_ex])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
