dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2016

AC_DEFUN([LTP_CHECK_BUILTIN_CLEAR_CACHE],[
	AC_MSG_CHECKING([for __builtin___clear_cache])
	AC_LINK_IFELSE([AC_LANG_SOURCE([[
int main(void) {
	char arr[16];
	__builtin___clear_cache(arr, arr + sizeof(arr));
        return 0;
}]])],[has_bcc="yes"])

if test "x$has_bcc" = xyes; then
	AC_DEFINE(HAVE_BUILTIN_CLEAR_CACHE,1,[Define to 1 if you have __builtin___clear_cache])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
