dnl
dnl Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
dnl LTP_CHECK_FORTIFY_SOURCE
dnl ------------------------
dnl
AC_DEFUN([LTP_CHECK_FORTIFY_SOURCE],[dnl
	AC_MSG_CHECKING(whether to define _FORTIFY_SOURCE=2)
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>

int main(void)
{
#if !defined _FORTIFY_SOURCE && defined __OPTIMIZE__ && __OPTIMIZE__
	return 0;
#else
# error Compiling without optimalizations
#endif
}
])],[CPPFLAGS="$CPPFLAGS -D_FORTIFY_SOURCE=2"
AC_MSG_RESULT(yes)],
[AC_MSG_RESULT(no)])
])
