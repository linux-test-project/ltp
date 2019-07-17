dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2015

AC_DEFUN([LTP_CHECK_IF_LINK],[
AC_CHECK_DECLS([IFLA_NET_NS_PID],,,[#include <linux/if_link.h>])
])
