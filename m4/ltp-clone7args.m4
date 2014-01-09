dnl
dnl Copyright (c) Linux Test Project, 2014
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
dnl LTP_CHECK_CLONE_SUPPORTS_7_ARGS
dnl -------------------------------
dnl
AC_DEFUN([LTP_CHECK_CLONE_SUPPORTS_7_ARGS],[
AH_TEMPLATE(CLONE_SUPPORTS_7_ARGS,
[Define to 1 if clone() supports 7 arguments.])
AC_MSG_CHECKING([if clone() supports 7 args])
AC_TRY_LINK([#define _GNU_SOURCE
		#include <sched.h>
		#include <stdlib.h>],
		[
		#ifndef __ia64__
		clone(NULL, NULL, 0, NULL, NULL, NULL, NULL);
		#endif
		],
		AC_DEFINE(CLONE_SUPPORTS_7_ARGS) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
])
