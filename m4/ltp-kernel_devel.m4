dnl Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License as
dnl published by the Free Software Foundation; either version 2 of
dnl the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it would be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write the Free Software Foundation,
dnl Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
dnl
dnl Author: Alexey Kodanev <alexey.kodanev@oracle.com>
dnl

dnl
dnl LTP_CHECK_KERNEL_DEVEL
dnl ----------------------------
dnl Building kernel modules
dnl requires kernel-devel installed
dnl

AC_DEFUN([LTP_CHECK_KERNEL_DEVEL],[dnl

AC_MSG_CHECKING([for kernel-devel])
AC_ARG_WITH(
	[linux-version],
	[AC_HELP_STRING([--with-linux-version=VERSION],
			[specify the Linux version to build modules for])],
	[LINUX_VERSION="${withval}"],
	AS_IF([test "$cross_compiling" = "no"],
		[LINUX_VERSION=`uname -r`]))

AC_SUBST(LINUX_VERSION)

AC_ARG_WITH([linux-dir],
	[AC_HELP_STRING([--with-linux-dir=DIR],
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
	[AC_HELP_STRING([--without-modules],
			[disable auto-building kernel modules])],
			[WITH_MODULES="no"],
			[])

AC_SUBST(WITH_MODULES)
])
