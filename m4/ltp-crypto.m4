dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2009-2017

AC_DEFUN([LTP_CHECK_CRYPTO], [
	AC_CHECK_LIB([crypto], [SHA1_Init], [have_libcrypto=yes], [AC_MSG_WARN(missing libcrypto)])
	AC_CHECK_HEADERS([openssl/sha.h], [have_sha=yes], [AC_MSG_WARN(missing openssl headers)])
	if test "x$have_libcrypto" = "xyes" -a "x$have_sha" = "xyes"; then
		AC_DEFINE(HAVE_LIBCRYPTO, 1, [Define whether libcrypto and openssl headers are installed])
		AC_SUBST(CRYPTO_LIBS, "-lcrypto")
	fi
])
