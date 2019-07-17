dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Jiri Palecek 2009

AC_DEFUN([LTP_CHECK_LINUX_PTRACE],
_LTP_CHECK_LINUX_PTRACE
)

dnl Check for ptrace support
dnl in commit 016ae219 in July 2008
AC_DEFUN([_LTP_CHECK_LINUX_PTRACE],[
dnl order of headers checked here is significant
AC_CHECK_HEADERS([ \
	sys/ptrace.h \
	sys/reg.h \
	asm/ptrace.h \
	linux/ptrace.h \
])
save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS -I$srcdir/testcases/kernel/syscalls/ptrace"
AC_CHECK_TYPES([struct user_regs_struct, struct pt_regs],,,[#include "ptrace.h"])
AC_CHECK_DECLS([PTRACE_GETSIGINFO, PTRACE_O_TRACEVFORKDONE, PTRACE_SETOPTIONS],,,[#include "ptrace.h"])
dnl glibc-2.18 defines ptrace_peeksiginfo_args in sys/ptrace.h which
dnl conflicts with the one from linux kernel in linux/ptrace.h
AC_CHECK_TYPES([struct ptrace_peeksiginfo_args],,,[#include <sys/ptrace.h>])
CPPFLAGS=$save_CPPFLAGS
])
