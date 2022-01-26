AC_PREREQ(2.61)
AC_INIT([realtime],[realtime-20090930],[ltp-results@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([include/realtime_config.h])
AC_CONFIG_FILES([   \
	m4/Makefile \
])
AC_CHECK_HEADERS_ONCE([ \
	pthread.h \
])
AC_CANONICAL_HOST

AC_PROG_CC
# <= autoconf 2.61 doesn't have AC_PROG_AR, but 2.63 has it. Not sure about
# 2.62.
AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
AC_PROG_AR
AC_PROG_RANLIB

AC_MSG_CHECKING([for __sync_add_and_fetch gcc builtin function])
AC_LINK_IFELSE([AC_LANG_SOURCE([
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
int main(void) {
	char *c;
	return __sync_add_and_fetch(c, 1);
}])],[has___sync_add_and_fetch=yes])
if test "x$has___sync_add_and_fetch" = xyes; then
	AC_DEFINE(HAVE___SYNC_ADD_AND_FETCH,1,[Define to 1 if you have the __sync_add_and_fetch gcc builtin function])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

AC_CHECK_LIB([m], [exp10], [AC_DEFINE([HAVE_EXP10], 1, [Define to 1 if you have exp10 function])])

REALTIME_CHECK_PRIO_INHERIT

AC_OUTPUT
