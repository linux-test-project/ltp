dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2016
dnl Copyright (c) Linux Test Project, 2025

AC_DEFUN([LTP_CHECK_CLEAR_CACHE],[
	AC_MSG_CHECKING([for __clear_cache])
	AC_LINK_IFELSE([AC_LANG_SOURCE([[
int main(void) {
	char arr[16];
	__clear_cache(arr, arr + sizeof(arr));
        return 0;
}]])],[has_clear_cache="yes"])

if test "x$has_clear_cache" = xyes; then
	AC_DEFINE(HAVE_CLEAR_CACHE, 1, [Define to 1 if you have __clear_cache])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
