dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Red Hat Inc., 2008
dnl Copyright (c) 2019 Fujitsu Ltd.
dnl Author: Masatake YAMATO <yamato@redhat.com>

AC_DEFUN([LTP_CHECK_SYSCALL_SIGNALFD],[

AC_CHECK_FUNCS(signalfd,,)
AC_CHECK_HEADERS([sys/signalfd.h],,)
AC_CHECK_HEADERS([linux/signalfd.h],,)
AC_CHECK_MEMBERS([struct signalfd_siginfo.ssi_signo],,,[
#if defined HAVE_SYS_SIGNALFD_H
#include <sys/signalfd.h>
#elif defined HAVE_LINUX_SIGNALFD_H
#include <linux/signalfd.h>
#endif])
])
