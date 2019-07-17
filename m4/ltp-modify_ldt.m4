dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Red Hat Inc., 2008
dnl Author: Masatake YAMATO <yamato@redhat.com>

AC_DEFUN([LTP_CHECK_SYSCALL_MODIFY_LDT],
[
AC_CHECK_FUNCS(modify_ldt)
AC_CHECK_HEADERS(asm/ldt.h,[LTP_SYSCALL_MODIFY_LDT_HEADER=yes])
if test x"$LTP_SYSCALL_MODIFY_LDT_HEADER" = xyes; then
   AC_CHECK_TYPES([struct user_desc, struct modify_ldt_ldt_s],[],[],[
#include <asm/ldt.h>
])
fi])
