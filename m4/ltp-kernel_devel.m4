dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
dnl Author: Alexey Kodanev <alexey.kodanev@oracle.com>
dnl Building kernel modules
dnl kernel development headers installed

AC_DEFUN([LTP_CHECK_KERNEL_DEVEL],[

AC_MSG_CHECKING([for kernel-devel])
AC_ARG_WITH(
	[linux-version],
	[AS_HELP_STRING([--with-linux-version=VERSION],
			[specify the Linux version to build modules for])],
	[LINUX_VERSION="${withval}"],
	AS_IF([test "$cross_compiling" = "no"],
		[LINUX_VERSION=`uname -r`]))

AC_SUBST(LINUX_VERSION)

AC_ARG_WITH([linux-dir],
	[AS_HELP_STRING([--with-linux-dir=DIR],
			[specify path to kernel-devel directory])],
	[LINUX_DIR="${withval}"],
	AS_IF([test -n "$LINUX_VERSION"],
		[LINUX_DIR="/lib/modules/$LINUX_VERSION/build"]))

AC_SUBST(LINUX_DIR)

if test -f "$LINUX_DIR/Makefile"; then
	LINUX_VERSION_MAJOR=`make -C ${LINUX_DIR} -s kernelversion | cut -d. -f1`
	LINUX_VERSION_PATCH=`make -C ${LINUX_DIR} -s kernelversion | cut -d. -f2`
fi

if test -n "$LINUX_VERSION_MAJOR" -a -n "$LINUX_VERSION_PATCH"; then
	WITH_MODULES="yes"
else
	WITH_MODULES="no"
fi

AC_SUBST(LINUX_VERSION_MAJOR)
AC_SUBST(LINUX_VERSION_PATCH)

AC_MSG_RESULT([$WITH_MODULES])

AC_ARG_WITH(
	[modules],
	[AS_HELP_STRING([--without-modules],
			[disable auto-building kernel modules])],
			[WITH_MODULES="no"],
			[])

AC_SUBST(WITH_MODULES)
])
