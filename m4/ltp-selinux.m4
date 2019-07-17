dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Red Hat Inc., 2009

AC_DEFUN([LTP_CHECK_SELINUX],
[
AH_TEMPLATE(HAVE_LIBSELINUX_DEVEL,
[Define to 1 if you have both SELinux libraries and headers.])
AC_CHECK_HEADERS(selinux/selinux.h,[
        AC_CHECK_LIB(selinux,is_selinux_enabled,[
                AC_DEFINE(HAVE_LIBSELINUX_DEVEL) SELINUX_LIBS="-lselinux"],[
                SELINUX_LIBS=""])],[
        SELINUX_LIBS=""])
AC_SUBST(SELINUX_LIBS)])
