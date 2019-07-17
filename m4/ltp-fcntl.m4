dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2014 Fujitsu Ltd.
dnl Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>

AC_DEFUN([LTP_CHECK_SYSCALL_FCNTL],[
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
