AC_PREREQ([2.61])
AC_INIT([ltp], [cvs], [ltp-results@lists.sourceforge.net])
AC_CONFIG_HEADERS([include/config.h])


m4_include([m4/ltp-signalfd.m4])
LTP_CHECK_SYSCALL_SIGNALFD

m4_include([m4/ltp-modify_ldt.m4])
LTP_CHECK_SYSCALL_MODIFY_LDT


AC_OUTPUT
