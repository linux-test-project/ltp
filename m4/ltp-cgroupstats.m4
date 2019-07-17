dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2009

AC_DEFUN([LTP_CHECK_CGROUPSTATS],
[
AC_CHECK_HEADERS(linux/cgroupstats.h,[LTP_CHECK_CGROUPSTATS_HEADER=yes])
AC_SUBST(LTP_CHECK_CGROUPSTATS_HEADER)
])
