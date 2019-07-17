dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems, Inc. 2009
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

dnl LTP_CHECK_SIGNAL
dnl --------------------------
dnl * Check for sa_handler in struct_sigaction. The very fact that this
dnl   definition is present or missing signifies whether or not the rt_sig*
dnl   syscalls exist and are implemented on the target architecture, as the
dnl   sigaction(2) call obscures this point in glibc. This doesn't signify
dnl   whether or not the RT signals function though -- those must be proved
dnl   through functionality tests.
AC_DEFUN([LTP_CHECK_SIGNAL],
[
AC_CHECK_MEMBERS([struct sigaction.sa_sigaction],[],[],[
#include <signal.h>
])
])
