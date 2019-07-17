dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Red Hat Inc., 2008
dnl Author: Masatake YAMATO <yamato@redhat.com>

dnl LTP_CHECK_SYSCALL_SIGNALFD
dnl --------------------------
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
AC_DEFUN([LTP_CHECK_SYSCALL_SIGNALFD],
[
_LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION
_LTP_CHECK_SYSCALL_SIGNALFD_HEADER

if test x"$LTP_SYSCALL_SIGNALFD_HEADER" = xyes; then
   _LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX
fi]
)

dnl _LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION
dnl ------------------------------------
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_FUNCTION],[
AC_CHECK_FUNCS(signalfd,[LTP_SYSCALL_SIGNALFD_FUNCTION=yes])])

dnl _LTP_CHECK_SYSCALL_SIGNALFD_HEADER
dnl ----------------------------------
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_HEADER],
[
AC_CHECK_HEADERS([sys/signalfd.h linux/types.h])
AC_CHECK_HEADERS([linux/signalfd.h signalfd.h],[
LTP_SYSCALL_SIGNALFD_HEADER=yes],,[
#ifdef HAVE_LINUX_TYPES_H
#include <linux/types.h>
#endif
]
)
]
)

dnl _LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX
dnl ----------------------------------------
AC_DEFUN([_LTP_CHECK_SYSCALL_SIGNALFD_FIELD_PREFIX],
[
AC_CHECK_MEMBERS([struct signalfd_siginfo.ssi_signo, struct signalfd_siginfo.signo],[
LTP_SYSCALL_SIGNALFD_FIELD_PREFIX=given],,[
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
]
)
