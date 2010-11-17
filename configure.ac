AC_PREREQ(2.61)
AC_INIT([ltp], [LTP_VERSION], [ltp-results@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([ \
    include/mk/config.mk \
    include/mk/features.mk \
    lib/ltp.pc \
    ltp-devel.spec \
    m4/Makefile \
    execltp \
    README.ltp-devel \
])

AM_CONDITIONAL(CROSS_COMPILATION, test x$cross_compiling = xyes)

AC_CANONICAL_HOST

AC_PROG_CC
# <= autoconf 2.61 doesn't have AC_PROG_AR, but 2.63 has it. Not sure about
# 2.62.
AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
AC_PROG_AR
AC_PROG_LEX
AC_PROG_RANLIB
AC_DEFUN([AC_PROG_STRIP], [AC_CHECK_TOOL(STRIP, strip, :)])
AC_PROG_STRIP
AC_PROG_YACC

# NOTE (garrcoop): was in use by testcases/DOTS.
# Code mostly borrowed from
# http://mountain.nsu.ru/doc/autoconf-archive/html/ac_prog_javac.html
#AC_DEFUN([AC_PROG_JAVAC],[
#AC_REQUIRE([AC_EXEEXT])dnl
#if test "x$JAVAPREFIX" = x; then
#	test "x$JAVAC" = x && AC_CHECK_PROGS(JAVAC, "gcj$EXEEXT -C" guavac$EXEEXT jikes$EXEEXT javac$EXEEXT)
#else
#	test "x$JAVAC" = x && AC_CHECK_PROGS(JAVAC, "gcj$EXEEXT -C" guavac$EXEEXT jikes$EXEEXT javac$EXEEXT, $JAVAPREFIX)
#fi
#if test "x$JAVAC" = x; then
#	AC_MSG_WARN([no acceptable Java compiler found in \$PATH])
#fi
#AC_PROVIDE([$0])dnl
#])
#AC_PROG_JAVAC

AC_PREFIX_DEFAULT(/opt/ltp)

AC_CHECK_HEADERS([ \
    dmapi.h \
    ifaddrs.h \
    libaio.h \
    mm.h \
    pthread.h \
    linux/genetlink.h \
    linux/mempolicy.h \
    linux/module.h \
    linux/netlink.h \
    sys/acl.h \
    sys/epoll.h \
    sys/inotify.h \
    sys/jfsdmapi.h \
    sys/prctl.h \
])

AC_CONFIG_SUBDIRS([ \
    testcases/kernel/syscalls/libevent \
])

AC_ARG_WITH([open-posix-testsuite],
  [AC_HELP_STRING([--with-open-posix-testsuite],
    [compile and install the open posix testsuite (default=no)])],
  [with_open_posix_testsuite=yes]
)
if test "x$with_open_posix_testsuite" = xyes; then
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["yes"])
else
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["no"])
fi

AC_ARG_WITH([realtime-testsuite],
  [AC_HELP_STRING([--with-realtime-testsuite],
    [compile and install the realtime testsuite (default=no)])],
  [with_realtime_testsuite=yes]
)
if test "x$with_realtime_testsuite" = xyes; then
    AC_SUBST([WITH_REALTIME_TESTSUITE],["yes"])
else
    AC_SUBST([WITH_REALTIME_TESTSUITE],["no"])
fi

if test "x$with_realtime_testsuite" = xyes; then
AC_CONFIG_SUBDIRS([ \
    testcases/realtime \
])
fi

LTP_CHECK_CAPABILITY_SUPPORT
LTP_CHECK_CRYPTO
LTP_CHECK_LINUX_PTRACE
LTP_CHECK_NOMMU_LINUX
LTP_CHECK_PRCTL_SUPPORT
LTP_CHECK_SECUREBITS
LTP_CHECK_SELINUX
LTP_CHECK_SIGNAL
LTP_CHECK_SYSCALL_EVENTFD
LTP_CHECK_SYSCALL_KEYCTL
LTP_CHECK_SYSCALL_MODIFY_LDT
LTP_CHECK_SYSCALL_NUMA
LTP_CHECK_SYSCALL_QUOTACTL
LTP_CHECK_SYSCALL_SIGNALFD
LTP_CHECK_SYSCALL_UNSHARE
LTP_CHECK_TASKSTATS
LTP_CHECK_TIME

AC_OUTPUT
