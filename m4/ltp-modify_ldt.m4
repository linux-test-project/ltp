dnl
dnl Copyright (c) Red Hat Inc., 2008
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
dnl Author: Masatake YAMATO <yamato@redhat.com>
dnl

dnl
dnl LTP_CHECK_SYSCALL_MODIFY_LDT
dnl ----------------------------
dnl
AC_DEFUN([LTP_CHECK_SYSCALL_MODIFY_LDT],
[dnl
AC_CHECK_FUNCS(modify_ldt)
AC_CHECK_HEADERS(asm/ldt.h,[LTP_SYSCALL_MODIFY_LDT_HEADER=yes])
if test x"$LTP_SYSCALL_MODIFY_LDT_HEADER" = xyes; then
   AC_CHECK_TYPES([struct user_desc, struct modify_ldt_ldt_s],[],[],[dnl
#include <asm/ldt.h>
])
fi])dnl
