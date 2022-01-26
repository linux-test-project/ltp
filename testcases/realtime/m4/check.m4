AC_DEFUN([REALTIME_CHECK_PRIO_INHERIT],[
AC_MSG_CHECKING([for PTHREAD_PRIO_INHERIT])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <pthread.h>]], [[int main(void) {
	pthread_mutexattr_t attr;
	return pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
}]])],[has_priority_inherit="yes"],[])
if test "x$has_priority_inherit" = "xyes" ; then
	AC_DEFINE(HAS_PRIORITY_INHERIT,1,[Define to 1 if you have PTHREAD_PRIO_INHERIT])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
])
