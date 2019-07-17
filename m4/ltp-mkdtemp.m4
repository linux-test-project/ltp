dnl Copyright (c) Linux Test Project, 2011-2012
dnl Author: Cyril Hrubis <chrubis@suse.cz>

AC_DEFUN([LTP_CHECK_MKDTEMP],[
AC_CHECK_FUNCS(mkdtemp,[],AC_MSG_ERROR(mkdtemp() not found!))])
