dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
dnl Author: Alexey Kodanev <alexey.kodanev@oracle.com>

AC_DEFUN([LTP_CHECK_CC_WARN_OLDSTYLE],[

wflag="-Wold-style-definition"
AC_MSG_CHECKING([if $CC supports $wflag])

backup_cflags="$CFLAGS"
CFLAGS="$CFLAGS $wflag"

AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([])],
	[GCC_WARN_OLDSTYLE="$wflag"]
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])]
)

AC_SUBST(GCC_WARN_OLDSTYLE)
CFLAGS="$backup_cflags"

])
