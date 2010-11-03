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

# Code mostly borrowed from
# http://mountain.nsu.ru/doc/autoconf-archive/html/ac_prog_javac.html
AC_DEFUN([AC_PROG_JAVAC],[
AC_REQUIRE([AC_EXEEXT])dnl
if test "x$JAVAPREFIX" = x; then
	test "x$JAVAC" = x && AC_CHECK_PROGS(JAVAC, "gcj$EXEEXT -C" guavac$EXEEXT jikes$EXEEXT javac$EXEEXT)
else
	test "x$JAVAC" = x && AC_CHECK_PROGS(JAVAC, "gcj$EXEEXT -C" guavac$EXEEXT jikes$EXEEXT javac$EXEEXT, $JAVAPREFIX)
fi
if test "x$JAVAC" = x; then
	AC_MSG_WARN([no acceptable Java compiler found in \$PATH])
fi
AC_PROVIDE([$0])dnl
])
AC_PROG_JAVAC


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

# open_hpi_testsuite does NOT fit cleanly in the build. BLAH.
#AC_CONFIG_SUBDIRS([ \
#    testcases/kernel/syscalls/libevent \
#    testcases/open_hpi_testsuite/ \
#    testcases/kernel/syscalls/pcllib/ \
#    testcases/realtime \
#])

AC_CONFIG_SUBDIRS([ \
    testcases/kernel/syscalls/libevent \
    testcases/kernel/syscalls/pcllib/ \
    testcases/realtime \
])

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
