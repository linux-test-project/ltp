AC_PREREQ([2.61])
AC_INIT([ltp], [cvs], [ltp-results@lists.sourceforge.net])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([config.mk])


m4_include([m4/ltp-signalfd.m4])
LTP_CHECK_SYSCALL_SIGNALFD

m4_include([m4/ltp-modify_ldt.m4])
LTP_CHECK_SYSCALL_MODIFY_LDT

AC_CHECK_HEADERS([sys/inotify.h])

AC_CHECK_HEADERS([sys/capability.h])

m4_include([m4/ltp-eventfd.m4])
LTP_CHECK_SYSCALL_EVENTFD

AC_CHECK_HEADERS([sys/inotify.h])

AC_CHECK_HEADERS([ifaddrs.h])

AC_OUTPUT
