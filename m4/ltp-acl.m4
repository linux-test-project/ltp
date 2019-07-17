dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Linux Test Project, 2011
dnl Author: Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_ACL_SUPPORT], [
	AC_CHECK_LIB([acl], [acl_init], [have_libacl=yes], [AC_MSG_WARN(missing libacl)])
	AC_CHECK_HEADERS([sys/acl.h], [have_acl=yes], [AC_MSG_WARN(missing libacl headers)])
	if test "x$have_libacl" = "xyes" -a "x$have_acl" = "xyes"; then
		AC_DEFINE(HAVE_LIBACL, 1, [Define to 1 if you have libacl and it's headers installed])
	    AC_SUBST(ACL_LIBS, "-lacl")
	fi
])
