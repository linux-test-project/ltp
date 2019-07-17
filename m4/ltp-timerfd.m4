dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2014

AC_DEFUN([LTP_CHECK_TIMERFD],[
AC_CHECK_FUNCS([timerfd_create timerfd_settime timerfd_gettime])
AC_CHECK_HEADERS([sys/timerfd.h])
])
