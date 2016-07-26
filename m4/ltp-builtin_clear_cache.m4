dnl
dnl Copyright (c) Linux Test Project, 2016
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

AC_DEFUN([LTP_CHECK_BUILTIN_CLEAR_CACHE],[dnl
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
