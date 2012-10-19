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
dnl LTP_CHECK_SYSCALL_SIGNALFD
dnl --------------------------
dnl
dnl * Checking the existence of the libc wrapper for signalfd.
dnl   If it exists, a shell variable LTP_SYSCALL_SIGNALFD_FUNCTION is set to "yes".
dnl
dnl * Checking the existence of signalfd.h.
dnl   If it exists, a shell variable LTP_SYSCALL_SIGNALFD_HEADER is set to "yes".
dnl
dnl * Checking the prefix used in fileds for signalfd_siginfo structure.
dnl   If it exists, a shell variable LTP_SYSCALL_SIGNALFD_FIELD_PREFIX is set to "given".
dnl
dnl About cpp macros defined in this macro,
dnl see testcases/kernel/syscalls/signalfd/signalfd01.c of ltp.
dnl

AC_DEFUN([LTP_CHECK_SYSCALL_SIGNALFD],
[dnl
_LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION
_LTP_CHECK_SYSCALL_SIGNALFD_HEADER

if test x"$LTP_SYSCALL_SIGNALFD_HEADER" = xyes; then
   _LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX
fi]dnl
)dnl

dnl _LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION
dnl ------------------------------------
dnl
dnl
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION],[dnl
AC_CHECK_FUNCS(signalfd,[LTP_SYSCALL_SIGNALFD_FUNCTION=yes])])

dnl _LTP_CHECK_SYSCALL_SIGNALFD_HEADER
dnl ----------------------------------
dnl
dnl
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_HEADER],
[dnl
AC_CHECK_HEADERS([sys/signalfd.h linux/types.h])
AC_CHECK_HEADERS([linux/signalfd.h signalfd.h],[dnl
LTP_SYSCALL_SIGNALFD_HEADER=yes],,[dnl
#ifdef HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif
]dnl
)dnl
]dnl
)dnl

dnl _LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX
dnl ----------------------------------------
dnl
dnl
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX],
[dnl
AC_CHECK_MEMBERS([struct signalfd_siginfo.ssi_signo, struct signalfd_siginfo.signo],[dnl
LTP_SYSCALL_SIGNALFD_FIELD_PREFIX=given],,[dnl
#if defined HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#elif defined HAVE_LINUX_SIGNALFD_H
#ifdef HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif
#include <linux/signalfd.h>
#elif defined HAVE_SIGNALFD_H
#include <signalfd.h>
#endif])
]dnl
)dnl
