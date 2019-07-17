dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Serge Hallyn (2010)

AC_DEFUN([LTP_CHECK_SECUREBITS],[
	AC_CHECK_HEADERS(linux/securebits.h,[have_securebits=yes])
if test "x$have_securebits" != xyes; then
	have_securebits=no
fi
AC_SUBST(HAVE_SECUREBITS,$have_securebits)
])
