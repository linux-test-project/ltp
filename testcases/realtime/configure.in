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
AM_CONDITIONAL(CROSS_COMPILATION, test x$cross_compiling = xyes)

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

REALTIME_CHECK_PRIO_INHERIT

if test "x$has_priority_inherit" = xyes; then

	AC_MSG_CHECKING([for pthread mutex attr protocol functions])
	AC_TRY_COMPILE([
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>],[int main(void) {
	pthread_mutexattr_t attr;
	(void) pthread_mutexattr_getprotocol(&attr, (void*) NULL);
	(void) pthread_mutexattr_setprotocol(&attr, 0);
	return 0;
}],[has_pthread_mutexattr_protocol_functions="yes"])
fi
if test "x$has_pthread_mutexattr_protocol_functions" = "xyes" ; then
	AC_DEFINE(HAS_PTHREAD_MUTEXATTR_PROTOCOL_FUNCTIONS,1,[Define to 1 if you have the pthread_mutexattr protocol APIs])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
REALTIME_CHECK_ROBUST_APIS

LTP_CHECK_EXP10

AC_OUTPUT
