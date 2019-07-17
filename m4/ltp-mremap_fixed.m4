dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2012
dnl Author: Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_MREMAP_FIXED],[
AH_TEMPLATE(HAVE_MREMAP_FIXED,
[Define to 1 if you have MREMAP_FIXED in <sys/mman.h>.])
AC_MSG_CHECKING([for MREMAP_FIXED in <sys/mman.h>])
AC_TRY_COMPILE([#define _GNU_SOURCE
                #include <sys/mman.h>], [int flags = MREMAP_FIXED;],
               AC_DEFINE(HAVE_MREMAP_FIXED) AC_MSG_RESULT(yes), AC_MSG_RESULT(no))
])
