AC_PREREQ([2.61])
AC_INIT([ltp], [LTP_VERSION], [ltp-results@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([ \
    include/mk/config.mk \
    lib/ltp.pc \
    ltp-devel.spec \
    m4/Makefile \
    README.ltp-devel \
])

AM_CONDITIONAL(CROSS_COMPILATION, test x$cross_compiling = xyes)

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_AR
AC_PROG_LEX
AC_PROG_RANLIB
AC_PROG_YACC

AC_PREFIX_DEFAULT(/opt/ltp)

AC_CHECK_HEADERS_ONCE([ \
    dmapi.h \
    ifaddrs.h \
    libaio.h \
    mm.h \
    linux/genetlink.h \
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
#    testcases/open_hpi_testsuite/ \
#    testcases/kernel/syscalls/pcllib/ \
#])

AC_CONFIG_SUBDIRS([ \
    testcases/kernel/syscalls/pcllib/ \
])

LTP_CHECK_CAPABILITY_SUPPORT
LTP_CHECK_CRYPTO
LTP_CHECK_LINUX_PTRACE
LTP_CHECK_SELINUX
LTP_CHECK_SYSCALL_EVENTFD
LTP_CHECK_SYSCALL_MODIFY_LDT
LTP_CHECK_SYSCALL_NUMA
LTP_CHECK_SYSCALL_QUOTACTL
LTP_CHECK_SYSCALL_SIGNALFD
LTP_CHECK_SYSCALL_UNSHARE
LTP_CHECK_TASKSTATS

AC_OUTPUT
