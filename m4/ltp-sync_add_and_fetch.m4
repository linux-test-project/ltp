dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2016

AC_DEFUN([LTP_CHECK_SYNC_ADD_AND_FETCH],[
	AC_MSG_CHECKING([for __sync_add_and_fetch])
	AC_LINK_IFELSE([AC_LANG_SOURCE([
int main(void) {
	int i = 0;
	return __sync_add_and_fetch(&i, 1);
}])],[has_saac="yes"])

if test "x$has_saac" = xyes; then
	AC_DEFINE(HAVE_SYNC_ADD_AND_FETCH,1,[Define to 1 if you have __sync_add_and_fetch])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
