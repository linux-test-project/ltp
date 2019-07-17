dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2011
dnl Author: Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_MADVISE],
[
AC_CHECK_DECLS([MADV_MERGEABLE],[have_madv_mergeable="yes"],,[#include <sys/mman.h>])
if test "x$have_madv_mergeable" = "xyes"; then
	AC_DEFINE(HAVE_MADV_MERGEABLE,1,[Define to 1 if you have MADV_MERGEABLE])
fi
])
