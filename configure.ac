AC_PREREQ(2.61)
AC_INIT([ltp], [LTP_VERSION], [ltp@lists.linux.it])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([ \
    include/mk/config.mk \
    include/mk/config-openposix.mk \
    include/mk/features.mk \
    lib/ltp.pc \
    m4/Makefile \
    execltp \
])

AM_MAINTAINER_MODE([enable])

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

AC_PREFIX_DEFAULT(/opt/ltp)

AC_CHECK_DECLS([IFLA_NET_NS_PID],,,[#include <linux/if_link.h>])
AC_CHECK_DECLS([MADV_MERGEABLE],,,[#include <sys/mman.h>])
AC_CHECK_DECLS([PR_CAPBSET_DROP, PR_CAPBSET_READ],,,[#include <sys/prctl.h>])

AC_CHECK_HEADERS([ \
    asm/ldt.h \
    ifaddrs.h \
    keyutils.h \
    linux/can.h \
    linux/cgroupstats.h \
    linux/cryptouser.h \
    linux/dccp.h \
    linux/fs.h \
    linux/genetlink.h \
    linux/if_alg.h \
    linux/if_ether.h \
    linux/if_packet.h \
    linux/keyctl.h \
    linux/mempolicy.h \
    linux/module.h \
    linux/netlink.h \
    linux/seccomp.h \
    linux/securebits.h \
    linux/tty.h \
    linux/types.h \
    linux/userfaultfd.h \
    mm.h \
    netinet/sctp.h \
    pthread.h \
    sys/epoll.h \
    sys/fanotify.h \
    sys/inotify.h \
    sys/prctl.h \
    sys/shm.h \
    sys/timerfd.h \
    sys/ustat.h \
    sys/utsname.h \
    sys/xattr.h \
    xfs/xqm.h \
])
AC_CHECK_HEADERS(fts.h, [have_fts=1])
AC_SUBST(HAVE_FTS_H, $have_fts)

AC_CHECK_FUNCS([ \
    clone3 \
    copy_file_range \
    epoll_pwait \
    execveat \
    fallocate \
    fchownat \
    fsconfig \
    fsmount \
    fsopen \
    fspick \
    fstatat \
    getdents \
    getdents64 \
    io_pgetevents \
    io_uring_setup \
    io_uring_register \
    io_uring_enter \
    kcmp \
    mkdirat \
    mknodat \
    modify_ldt \
    move_mount \
    name_to_handle_at \
    open_tree \
    openat \
    openat2 \
    pidfd_open \
    pidfd_send_signal \
    pkey_mprotect \
    preadv \
    preadv2 \
    profil \
    pwritev \
    pwritev2 \
    readlinkat \
    recvmmsg \
    renameat \
    renameat2 \
    sched_getcpu \
    sendmmsg \
    setns \
    sigpending \
    splice \
    statx \
    stime \
    sync_file_range \
    syncfs \
    tee \
    timerfd_create \
    timerfd_gettime \
    timerfd_settime \
    unshare \
    ustat \
    vmsplice \
])
AC_CHECK_FUNCS(mkdtemp,[],AC_MSG_ERROR(mkdtemp() not found!))

AC_CHECK_MEMBERS([struct fanotify_event_info_fid.fsid.__val],,,[#include <sys/fanotify.h>])
AC_CHECK_MEMBERS([struct perf_event_mmap_page.aux_head],,,[#include <linux/perf_event.h>])
AC_CHECK_MEMBERS([struct sigaction.sa_sigaction],[],[],[#include <signal.h>])

AC_CHECK_MEMBERS([struct utsname.domainname],,,[
#define _GNU_SOURCE
#include <sys/utsname.h>
])

AC_CHECK_TYPES([enum kcmp_type],,,[#include <linux/kcmp.h>])
AC_CHECK_TYPES([struct acct_v3],,,[#include <sys/acct.h>])
AC_CHECK_TYPES([struct fanotify_event_info_fid],,,[#include <sys/fanotify.h>])
AC_CHECK_TYPES([struct fanotify_event_info_header],,,[#include <sys/fanotify.h>])
AC_CHECK_TYPES([struct file_dedupe_range],,,[#include <linux/fs.h>])
AC_CHECK_TYPES([struct fs_quota_statv],,,[#include <xfs/xqm.h>])
AC_CHECK_TYPES([struct if_nextdqblk],,,[#include <linux/quota.h>])
AC_CHECK_TYPES([struct iovec],,,[#include <sys/uio.h>])

AC_CHECK_TYPES([struct mmsghdr],,,[
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_TYPES([struct rlimit64],,,[
#define _LARGEFILE64_SOURCE
#include <sys/resource.h>
])

AC_CHECK_TYPES([struct statx, struct statx_timestamp],,,[
#define _GNU_SOURCE
#include <sys/stat.h>
])

AC_CHECK_TYPES([struct termio],,,[#include <sys/ioctl.h>])

AC_CHECK_TYPES([struct tpacket_req3],,,[
#ifdef HAVE_LINUX_IF_PACKET_H
# include <linux/if_packet.h>
#endif
])

AC_CHECK_TYPES([struct user_desc, struct modify_ldt_ldt_s],[],[],[
#include <asm/ldt.h>
])

AC_CHECK_TYPES([struct xt_entry_match, struct xt_entry_target],,,[
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <limits.h>
#include <linux/netfilter_ipv4/ip_tables.h>
])

# Tools knobs

# Expect
AC_ARG_WITH([bash],
  [AC_HELP_STRING([--with-bash],
    [have the Bourne Again Shell interpreter])],
  [with_bash=$withval],
  [with_bash=no]
)
if test "x$with_bash" = xyes; then
    AC_SUBST([WITH_BASH],["yes"])
else
    AC_SUBST([WITH_BASH],["no"])
fi

AC_ARG_WITH([expect],
  [AC_HELP_STRING([--with-expect],
    [have the Tcl/expect library])],
  [with_expect=$withval],
  [with_expect=no]
)
if test "x$with_expect" = xyes; then
    AC_SUBST([WITH_EXPECT],["yes"])
else
    AC_SUBST([WITH_EXPECT],["no"])
fi

# Numa
AC_ARG_WITH([numa],
  AC_HELP_STRING([--without-numa],
    [without numa support]),
  [with_numa=$withval],
  [with_numa=yes]
)

# Perl
AC_ARG_WITH([perl],
  [AC_HELP_STRING([--with-perl],
    [have a perl interpreter])],
  [with_perl=$withval],
  [with_perl=no]
)
if test "x$with_perl" = xyes; then
    AC_SUBST([WITH_PERL],["yes"])
else
    AC_SUBST([WITH_PERL],["no"])
fi

# Python
AC_ARG_WITH([python],
  [AC_HELP_STRING([--with-python],
    [have a python interpreter])],
  [with_python=$withval],
  [with_python=no]
)
if test "x$with_python" = xyes; then
    AC_SUBST([WITH_PYTHON],["yes"])
else
    AC_SUBST([WITH_PYTHON],["no"])
fi

# TI RPC
AC_ARG_WITH([tirpc],
  AC_HELP_STRING([--without-tirpc],
    [without libtirpc support]),
  [with_tirpc=$withval],
  [with_tirpc=yes]
)
# END tools knobs

# Testsuites knobs

AC_ARG_WITH([open-posix-testsuite],
  [AC_HELP_STRING([--with-open-posix-testsuite],
    [compile and install the open posix testsuite])],
  [with_open_posix_testsuite=$withval],
  [with_open_posix_testsuite=no]
)
if test "x$with_open_posix_testsuite" = xyes; then
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["yes"])
else
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["no"])
fi

# TODO: testcases/realtime requires bash and python.
AC_ARG_WITH([realtime-testsuite],
  [AC_HELP_STRING([--with-realtime-testsuite],
    [compile and install the realtime testsuite])],
  [with_realtime_testsuite=$withval],
  [with_realtime_testsuite=no]
)

if test "x$with_realtime_testsuite" = xyes; then
    AC_SUBST([WITH_REALTIME_TESTSUITE],["yes"])
    # Run configure on testcases/realtime as well.
    AC_CONFIG_SUBDIRS([testcases/realtime])
else
    AC_SUBST([WITH_REALTIME_TESTSUITE],["no"])
fi

AC_CONFIG_COMMANDS([syscalls.h], [cd ${ac_top_srcdir}/include/lapi/syscalls; ./regen.sh])

# custom functions
# NOTE: don't create custom functions for simple checks, put them into this file
LTP_CHECK_ACL_SUPPORT
LTP_CHECK_ATOMIC_MEMORY_MODEL
LTP_CHECK_BUILTIN_CLEAR_CACHE
LTP_CHECK_CAPABILITY_SUPPORT
LTP_CHECK_CC_WARN_OLDSTYLE
LTP_CHECK_CLONE_SUPPORTS_7_ARGS
LTP_CHECK_CRYPTO
LTP_CHECK_FORTIFY_SOURCE
LTP_CHECK_KERNEL_DEVEL
LTP_CHECK_KEYUTILS_SUPPORT
LTP_CHECK_LIBMNL
LTP_CHECK_LINUX_PTRACE
LTP_CHECK_LINUXRANDOM
LTP_CHECK_MREMAP_FIXED
LTP_CHECK_NOMMU_LINUX
LTP_CHECK_SELINUX
LTP_CHECK_SYNC_ADD_AND_FETCH
LTP_CHECK_SYSCALL_EVENTFD
LTP_CHECK_SYSCALL_FCNTL

if test "x$with_numa" = xyes; then
	LTP_CHECK_SYSCALL_NUMA
	numa_error_msg="test requires libnuma >= 2 and it's development packages"
else
	numa_error_msg="NUMA support was disabled during build"
fi
AC_DEFINE_UNQUOTED(NUMA_ERROR_MSG, ["$numa_error_msg"], [Error message when no NUMA support])


LTP_CHECK_SYSCALL_PERF_EVENT_OPEN
LTP_CHECK_SYSCALL_SIGNALFD
LTP_CHECK_SYSCALL_UTIMENSAT
LTP_CHECK_TASKSTATS
test "x$with_tirpc" = xyes && LTP_CHECK_TIRPC
LTP_DETECT_HOST_CPU

AC_OUTPUT
