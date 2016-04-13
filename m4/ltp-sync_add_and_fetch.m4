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

AC_DEFUN([LTP_CHECK_SYNC_ADD_AND_FETCH],[dnl
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
