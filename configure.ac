AC_PREREQ(2.64)
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
])

AC_ARG_VAR(HOSTCC, [The C compiler on the host])

AM_MAINTAINER_MODE([enable])

AC_CANONICAL_HOST

AC_PROG_CC
# <= autoconf 2.61 doesn't have AC_PROG_AR, but 2.63 has it. Not sure about
# 2.62.
AC_DEFUN([AC_PROG_AR], [AC_CHECK_TOOL(AR, ar, :)])
AC_PROG_AR
AC_PROG_RANLIB
AC_DEFUN([AC_PROG_STRIP], [AC_CHECK_TOOL(STRIP, strip, :)])
AC_PROG_STRIP
AC_PROG_YACC

m4_ifndef([PKG_CHECK_EXISTS],
	[m4_fatal([must install pkg-config or pkgconfig and pkg.m4 macro (usual dependency), see INSTALL])])

AC_PREFIX_DEFAULT(/opt/ltp)

AC_CHECK_DECLS([IFLA_NET_NS_PID],,,[#include <linux/if_link.h>])
AC_CHECK_DECLS([LANDLOCK_RULE_PATH_BENEATH],,,[#include <linux/landlock.h>])
AC_CHECK_DECLS([LANDLOCK_RULE_NET_PORT],,,[#include <linux/landlock.h>])
AC_CHECK_DECLS([MADV_MERGEABLE],,,[#include <sys/mman.h>])
AC_CHECK_DECLS([NFTA_CHAIN_ID, NFTA_VERDICT_CHAIN_ID],,,[#include <linux/netfilter/nf_tables.h>])
AC_CHECK_DECLS([PR_CAPBSET_DROP, PR_CAPBSET_READ],,,[#include <sys/prctl.h>])
AC_CHECK_DECLS([SEM_STAT_ANY],,,[#include <sys/sem.h>])

AC_CHECK_HEADERS_ONCE([ \
    asm/ldt.h \
    asm/prctl.h \
    cpuid.h \
    emmintrin.h \
    ifaddrs.h \
    keyutils.h \
    linux/can.h \
    linux/cgroupstats.h \
    linux/cryptouser.h \
    linux/close_range.h \
    linux/dccp.h \
    linux/futex.h \
    linux/genetlink.h \
    linux/if_alg.h \
    linux/if_ether.h \
    linux/if_packet.h \
    linux/io_uring.h \
    linux/ioprio.h \
    linux/keyctl.h \
    linux/landlock.h \
    linux/mempolicy.h \
    linux/module.h \
    linux/mount.h \
    linux/netlink.h \
    linux/seccomp.h \
    linux/securebits.h \
    linux/tls.h \
    linux/tty.h \
    linux/types.h \
    linux/userfaultfd.h \
    netinet/sctp.h \
    pthread.h \
    sys/epoll.h \
    sys/fanotify.h \
    sys/inotify.h \
    sys/pidfd.h
    sys/prctl.h \
    sys/random.h \
    sys/shm.h \
    sys/timerfd.h \
    sys/ustat.h \
    sys/utsname.h \
    sys/xattr.h \
    xfs/xqm.h \
])
AC_CHECK_HEADERS(fts.h, [have_fts=1])
AC_SUBST(HAVE_FTS_H, $have_fts)
AC_CHECK_HEADERS(linux/vm_sockets.h, [], [], [#include <sys/socket.h>])

AC_CHECK_FUNCS_ONCE([ \
    cachestat \
    clone3 \
    close_range \
    copy_file_range \
    epoll_pwait \
    epoll_pwait2 \
    execveat \
    faccessat2 \
    fallocate \
    fsconfig \
    fsmount \
    fsopen \
    fspick \
    fstatat \
    getauxval \
    getcontext \
    getcpu \
    getdents \
    getdents64 \
    io_pgetevents \
    io_uring_setup \
    io_uring_register \
    io_uring_enter \
    kcmp \
    mallinfo \
    mallinfo2 \
    mallopt \
    modify_ldt \
    mount_setattr \
    move_mount \
    name_to_handle_at \
    open_tree \
    openat \
    openat2 \
    pidfd_getfd \
    pidfd_open \
    pidfd_send_signal \
    pkey_mprotect \
    preadv \
    preadv2 \
    profil \
    pwritev \
    pwritev2 \
    quotactl_fd \
    rand_r \
    recvmmsg \
    renameat2 \
    sched_getcpu \
    sendmmsg \
    sethostid \
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
AC_CHECK_MEMBERS([struct statx.stx_mnt_id, struct statx.stx_dio_mem_align],,,[
#define _GNU_SOURCE
#include <sys/stat.h>
])

AC_CHECK_MEMBERS([struct utsname.domainname],,,[
#define _GNU_SOURCE
#include <sys/utsname.h>
])

AC_CHECK_TYPES([enum kcmp_type],,,[#include <linux/kcmp.h>])
AC_CHECK_TYPES([struct acct_v3],,,[#include <sys/acct.h>])
AC_CHECK_TYPES([struct af_alg_iv, struct sockaddr_alg],,,[# include <linux/if_alg.h>])
AC_CHECK_TYPES([struct fanotify_event_info_fid, struct fanotify_event_info_error,
		struct fanotify_event_info_header, struct fanotify_event_info_pidfd],,,[#include <sys/fanotify.h>])
AC_CHECK_TYPES([struct file_clone_range],,,[#include <linux/fs.h>])
AC_CHECK_TYPES([struct file_dedupe_range],,,[#include <linux/fs.h>])

AC_CHECK_TYPES([struct file_handle],,,[
#define _GNU_SOURCE
#include <fcntl.h>
])

AC_CHECK_TYPES([struct fs_quota_statv],,,[
#define _GNU_SOURCE
#include <xfs/xqm.h>
])

AC_CHECK_TYPES([struct if_nextdqblk],,,[#include <linux/quota.h>])
AC_CHECK_TYPES([struct iovec],,,[#include <sys/uio.h>])
AC_CHECK_TYPES([struct ipc64_perm],,,[#include <sys/ipcbuf.h>])
AC_CHECK_TYPES([struct loop_config],,,[#include <linux/loop.h>])
AC_CHECK_TYPES([struct landlock_path_beneath_attr],,,[#include <linux/landlock.h>])
AC_CHECK_TYPES([struct landlock_net_port_attr],,,[#include <linux/landlock.h>])

AC_CHECK_TYPES([struct mmsghdr],,,[
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
])

AC_CHECK_TYPES([struct msqid64_ds],,,[#include <sys/msgbuf.h>])

AC_CHECK_TYPES([struct rlimit64],,,[
#define _LARGEFILE64_SOURCE
#include <sys/resource.h>
])

AC_CHECK_TYPES([struct semid64_ds],,,[#include <sys/sem.h>])
AC_CHECK_TYPES([struct shmid64_ds],,,[#include <sys/shmbuf.h>])

AC_CHECK_TYPES([struct statx, struct statx_timestamp],,,[
#define _GNU_SOURCE
#include <sys/stat.h>
])

AC_CHECK_TYPES([struct termio],,,[#include <sys/ioctl.h>])
AC_CHECK_TYPES([struct tpacket_req3],,,[# include <linux/if_packet.h>])
AC_CHECK_TYPES([struct user_desc, struct modify_ldt_ldt_s],[],[],[#include <asm/ldt.h>])

AC_CHECK_TYPES([struct xt_entry_match, struct xt_entry_target],,,[
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <limits.h>
#include <linux/netfilter_ipv4/ip_tables.h>
])

AC_CHECK_TYPES([struct __kernel_old_timeval, struct __kernel_old_timespec, struct __kernel_timespec,
                struct __kernel_old_itimerspec, struct __kernel_itimerspec,
                struct __kernel_old_itimerval],,,[#include <sys/socket.h>])

AC_CHECK_TYPES([struct futex_waitv],,,[#include <linux/futex.h>])
AC_CHECK_TYPES([struct mount_attr],,,[
#ifdef HAVE_MOUNT_SETATTR
# include <sys/mount.h>
#elif HAVE_LINUX_MOUNT_H
# include <linux/mount.h>
#endif
])

AC_CHECK_TYPES([struct cachestat_range],,,[#include <sys/mman.h>])
AC_CHECK_TYPES([struct cachestat],,,[#include <sys/mman.h>])
AC_CHECK_TYPES([struct mnt_id_req],,,[#include <linux/mount.h>])
AC_CHECK_TYPES([struct statmount],,,[#include <linux/mount.h>])

# Tools knobs

# Bash
AC_ARG_WITH([bash],
  [AS_HELP_STRING([--with-bash],
    [have the Bourne Again Shell interpreter])],
  [with_bash=$withval],
  [with_bash=no]
)
if test "x$with_bash" = xyes; then
    AC_SUBST([WITH_BASH],["yes"])
else
    AC_SUBST([WITH_BASH],["no"])
fi

# metadata
AC_ARG_ENABLE([metadata],
  [AS_HELP_STRING([--disable-metadata],
	[Disable metadata generation (both HTML and PDF, default no)])],
  [], [enable_metadata=yes]
)
AC_ARG_ENABLE([metadata_html],
  [AS_HELP_STRING([--disable-metadata-html],
	[Disable metadata HTML generation (default no)])],
  [], [enable_metadata_html=yes]
)

AC_ARG_ENABLE([metadata_pdf],
  [AS_HELP_STRING([--enable-metadata-pdf],
	[Enable metadata PDF generation (default no)])],
  [], [enable_metadata_pdf=no]
)

AC_ARG_WITH([metadata_generator],
  [AS_HELP_STRING([--with-metadata-generator=asciidoc|asciidoctor],
	[Specify metadata generator to use (default autodetect)])],
  [with_metadata_generator=$withval],
  [with_metadata_generator=detect]
)

LTP_DOCPARSE

# Expect
AC_ARG_WITH([expect],
  [AS_HELP_STRING([--with-expect],
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
  AS_HELP_STRING([--without-numa],
    [without numa support]),
  [with_numa=$withval],
  [with_numa=yes]
)

# Perl
AC_ARG_WITH([perl],
  [AS_HELP_STRING([--with-perl],
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
  [AS_HELP_STRING([--with-python],
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
  AS_HELP_STRING([--without-tirpc],
    [without libtirpc support]),
  [with_tirpc=$withval],
  [with_tirpc=yes]
)
# END tools knobs

# Testsuites knobs

AC_ARG_WITH([open-posix-testsuite],
  [AS_HELP_STRING([--with-open-posix-testsuite],
    [compile and install the open posix testsuite])],
  [with_open_posix_testsuite=$withval],
  [with_open_posix_testsuite=no]
)

# Allow setting the directoy, where the open posix testsuite is installed to.
# If nothing is defined, we have to pass our default value to submake
AC_ARG_WITH([open-posix-testdir],
  [AS_HELP_STRING([--with-open-posix-testdir=<dir>],
    [set the directory, where the open posix testsuite will be installed under prefix])],
  [],
  [ac_configure_args="$ac_configure_args --with-open-posix-testdir=testcases/open_posix_testsuite"]
)

if test "x$with_open_posix_testsuite" = xyes; then
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["yes"])
    AC_CONFIG_SUBDIRS([testcases/open_posix_testsuite])
else
    AC_SUBST([WITH_OPEN_POSIX_TESTSUITE],["no"])
fi

# TODO: testcases/realtime requires bash and python.
AC_ARG_WITH([realtime-testsuite],
  [AS_HELP_STRING([--with-realtime-testsuite],
    [unused, kept for compatibility reason])],
  [with_realtime_testsuite=$withval],
  [with_realtime_testsuite=no]
)

if test "x$with_realtime_testsuite" = xyes; then
    AC_SUBST([WITH_REALTIME_TESTSUITE],["yes"])
else
    AC_SUBST([WITH_REALTIME_TESTSUITE],["no"])
fi

AC_CONFIG_COMMANDS([syscalls.h], [cd ${ac_top_srcdir}/include/lapi/syscalls; ./generate_syscalls.sh ../syscalls.h; cd - >/dev/null])

# custom functions
# NOTE: don't create custom functions for simple checks, put them into this file
LTP_CHECK_ACL_SUPPORT
LTP_CHECK_ATOMIC_MEMORY_MODEL
LTP_CHECK_BUILTIN_CLEAR_CACHE
LTP_CHECK_CAPABILITY_SUPPORT
LTP_CHECK_CC_WARN_OLDSTYLE
LTP_CHECK_CRYPTO
LTP_CHECK_FORTIFY_SOURCE
LTP_CHECK_KERNEL_DEVEL
LTP_CHECK_KEYUTILS_SUPPORT
LTP_CHECK_LIBMNL
LTP_CHECK_LINUXRANDOM
LTP_CHECK_SELINUX
LTP_CHECK_SYNC_ADD_AND_FETCH
LTP_CHECK_SYSCALL_EVENTFD
LTP_CHECK_SYSCALL_FCNTL
LTP_CHECK_FSVERITY

AX_CHECK_COMPILE_FLAG([-no-pie], [LTP_CFLAGS_NOPIE=1])
AX_CHECK_COMPILE_FLAG([-ffixed-ebp], [LTP_CFLAGS_FFIXED_EBP=1])
AC_SUBST([LTP_CFLAGS_NOPIE])
AC_SUBST([LTP_CFLAGS_FFIXED_EBP])

if test "x$with_numa" = xyes; then
	LTP_CHECK_SYSCALL_NUMA
	numa_error_msg="test requires libnuma development packages with LIBNUMA_API_VERSION >= 2"
else
	numa_error_msg="NUMA support was disabled during build"
fi
AC_DEFINE_UNQUOTED(NUMA_ERROR_MSG, ["$numa_error_msg"], [Error message when no NUMA support])


LTP_CHECK_SYSCALL_SIGNALFD
LTP_CHECK_SYSCALL_UTIMENSAT
LTP_CHECK_TASKSTATS
test "x$with_tirpc" = xyes && LTP_CHECK_TIRPC
LTP_DETECT_HOST_CPU

AC_MSG_CHECKING([whether linker can handle KVM payloads])
ltp_backup_ldflags="$LDFLAGS"
ltp_backup_flags="$[]_AC_LANG_PREFIX[]FLAGS"
LDFLAGS="-T ${srcdir}/testcases/kernel/kvm/linker/${HOST_CPU}.lds"
_AC_LANG_PREFIX[]FLAGS=
AC_LINK_IFELSE([AC_LANG_PROGRAM()],
  [
    AC_MSG_RESULT([yes])
    AC_SUBST([WITH_KVM_TESTSUITE],["yes"])
    have_kvm=yes
  ],
  [
    AC_MSG_RESULT([no])
    AC_SUBST([WITH_KVM_TESTSUITE],["no"])
    have_kvm=no
  ])
_AC_LANG_PREFIX[]FLAGS="$ltp_backup_flags"
LDFLAGS="$ltp_backup_ldflags"

AC_OUTPUT

cat << EOF

TESTSUITES
KVM testsuite: $have_kvm
open posix testsuite: ${with_open_posix_testsuite:-no}
TI-RPC testsuite: ${with_tirpc:-yes}

LIBRARIES
keyutils: ${have_keyutils:-yes}
libacl: ${have_libacl:-no}
libaio: ${have_libaio:-no} (aio: ${have_aio:-no})
libcap: $cap_libs (newer: ${has_newer_libcap:-no})
libcrypto: ${have_libcrypto:-no} (sha: ${have_sha:-no})
libmnl: ${have_libmnl:-yes}
libnuma: ${have_libnuma:-no} (headers: ${have_numa_headers:-yes}, v2 headers: ${have_numa_headers_v2:-no})
libtirpc: ${have_libtirpc:-no}
glibc SUN-RPC: ${have_rpc_glibc:-no}

METADATA
metadata generator: $with_metadata_generator
HTML metadata: $with_metadata_html
PDF metadata: $with_metadata_pdf
EOF

if test "x$with_realtime_testsuite" = xyes; then
    AC_MSG_WARN([--with-realtime-testsuite has no effect and is kept for compatibilty reason. It will be removed in the future.])
fi
