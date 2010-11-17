AC_DEFUN([REALTIME_CHECK_PRIO_INHERIT],[
AC_MSG_CHECKING([for PTHREAD_PRIO_INHERIT])
AC_TRY_COMPILE([
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>],[int main(void) {
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
}],[has_priority_inherit="yes"],[])
if test "x$has_priority_inherit" = "xyes" ; then
	AC_DEFINE(HAS_PRIORITY_INHERIT,1,[Define to 1 if you have PTHREAD_PRIO_INHERIT])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])

AC_DEFUN([REALTIME_CHECK_ROBUST_APIS],[
AC_MSG_CHECKING([for pthread_mutexattr_*robust* APIs])
AC_TRY_COMPILE([
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>],[int main(void) {
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setrobust_np(&attr, 0);
}],[has_robust="yes"])
if test "x$has_robust" = "xyes" ; then
	AC_DEFINE(HAS_PTHREAD_MUTEXTATTR_ROBUST_APIS,1,[Define to 1 if you have pthread_mutexattr_*robust* APIs])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])

AC_DEFUN([REALTIME_CHECK_ROBUST_APIS],[
AC_MSG_CHECKING([for pthread_mutexattr_*robust* APIs])
AC_TRY_COMPILE([
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>],[int main(void) {
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setrobust_np(&attr, 0);
}],[has_robust="yes"])
if test "x$has_robust" = "xyes" ; then
	AC_DEFINE(HAS_PTHREAD_MUTEXTATTR_ROBUST_APIS,1,[Define to 1 if you have pthread_mutexattr_*robust* APIs])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
