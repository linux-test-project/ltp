dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2016 Fujitsu Ltd.
dnl Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
dnl Author: Xiao Yang <yangx.jy@cn.fujitsu.com>

AC_DEFUN([LTP_CHECK_KEYUTILS_SUPPORT], [
	AC_CHECK_LIB([keyutils], [add_key],
	[AC_DEFINE(HAVE_LIBKEYUTILS, 1, [Define to 1 if you have libkeyutils installed.])
	      AC_SUBST(KEYUTILS_LIBS, "-lkeyutils")])
])
