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
dnl LTP_CHECK_EXP10
dnl ---------------
dnl
AC_DEFUN([LTP_CHECK_EXP10],[
AH_TEMPLATE(HAVE_EXP10,
[Define to 1 if you have 'exp10' function.])
AC_MSG_CHECKING([for exp10])
backup_ldlibs="$LIBS"
LIBS+=" -lm"
AC_TRY_LINK([#define _GNU_SOURCE
             #include <math.h>],
            [
             volatile float val;
             exp10(val);
            ],
             AC_DEFINE(HAVE_EXP10) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
LIBS="$backup_ldlibs"
])
