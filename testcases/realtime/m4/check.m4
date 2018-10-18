AC_DEFUN([REALTIME_CHECK_PRIO_INHERIT],[
AC_MSG_CHECKING([for PTHREAD_PRIO_INHERIT])
AC_TRY_COMPILE([
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
	AC_CHECK_DECLS([pthread_mutexattr_getrobust, pthread_mutexattr_setrobust],[],[has_robust="no"],[[#include <pthread.h>]])
	AC_MSG_CHECKING([for pthread_mutexattr_*robust* APIs])
if test "x$has_robust" != "xno"; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
