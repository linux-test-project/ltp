dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems Inc., 2009
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

AC_DEFUN([LTP_CHECK_PRCTL_SUPPORT],[
AC_CHECK_HEADERS(sys/prctl.h,[
	AC_CHECK_DECLS([PR_CAPBSET_DROP, PR_CAPBSET_READ], [],[],[
#include <sys/prctl.h>
]) dnl AC_CHECK_DECLS
])]
)
