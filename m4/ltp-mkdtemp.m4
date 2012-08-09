AC_DEFUN([LTP_CHECK_MKDTEMP],[
AC_CHECK_FUNCS(mkdtemp,[],AC_MSG_ERROR(mkdtemp() not found!))])
