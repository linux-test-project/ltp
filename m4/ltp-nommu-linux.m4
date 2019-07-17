dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2010
dnl Author: Mike Frysinger <vapier@gentoo.org>

AC_DEFUN([LTP_CHECK_NOMMU_LINUX],
[
	AC_CHECK_FUNCS([fork daemon vfork])
	UCLINUX=0
	if test "x$ac_cv_func_fork" = "xno" ; then
		UCLINUX=1
		AC_DEFINE([UCLINUX], 1, [Target is running Linux w/out an MMU])
	fi
	AC_SUBST(UCLINUX)
])
