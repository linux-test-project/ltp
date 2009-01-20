AC_PREREQ([2.61])
AC_INIT([ltp], [cvs], [ltp-results@lists.sourceforge.net])
AM_INIT_AUTOMAKE([1.10])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([config.mk m4/Makefile])

AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_AR
AC_PROG_RANLIB

AC_CHECK_HEADERS_ONCE([ \
ifaddrs.h \
sys/capability.h \
sys/inotify.h \
])

LTP_CHECK_SYSCALL_EVENTFD
LTP_CHECK_SYSCALL_MODIFY_LDT
LTP_CHECK_SYSCALL_SIGNALFD

AC_OUTPUT
