/************************************************
 * GENERATED FILE: DO NOT EDIT/PATCH THIS FILE  *
 *  change your arch specific .in file instead  *
 ************************************************/

/*
 * Here we stick all the ugly *fallback* logic for linux
 * system call numbers (those __NR_ thingies).
 *
 * Licensed under the GPLv2 or later, see the COPYING file.
 */

#ifndef __LINUX_SYSCALL_NUMBERS_H__
#define __LINUX_SYSCALL_NUMBERS_H__

#include <errno.h>
#include <sys/syscall.h>
#include "cleanup.c"

#define syscall(NR, ...) ({ \
	int __ret; \
	if (NR == 0) { \
		errno = ENOSYS; \
		__ret = -1; \
	} else { \
		__ret = syscall(NR, ##__VA_ARGS__); \
	} \
	if (__ret == -1 && errno == ENOSYS) { \
		tst_brkm(TCONF, CLEANUP, \
			"syscall " #NR " not supported on your arch"); \
		errno = ENOSYS; \
	} \
	__ret; \
})


#ifdef __arm__
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall (__NR_SYSCALL_BASE+ 0)
# endif
# ifndef __NR_exit
#  define __NR_exit (__NR_SYSCALL_BASE+ 1)
# endif
# ifndef __NR_fork
#  define __NR_fork (__NR_SYSCALL_BASE+ 2)
# endif
# ifndef __NR_read
#  define __NR_read (__NR_SYSCALL_BASE+ 3)
# endif
# ifndef __NR_write
#  define __NR_write (__NR_SYSCALL_BASE+ 4)
# endif
# ifndef __NR_open
#  define __NR_open (__NR_SYSCALL_BASE+ 5)
# endif
# ifndef __NR_close
#  define __NR_close (__NR_SYSCALL_BASE+ 6)
# endif
# ifndef __NR_creat
#  define __NR_creat (__NR_SYSCALL_BASE+ 8)
# endif
# ifndef __NR_link
#  define __NR_link (__NR_SYSCALL_BASE+ 9)
# endif
# ifndef __NR_unlink
#  define __NR_unlink (__NR_SYSCALL_BASE+ 10)
# endif
# ifndef __NR_execve
#  define __NR_execve (__NR_SYSCALL_BASE+ 11)
# endif
# ifndef __NR_chdir
#  define __NR_chdir (__NR_SYSCALL_BASE+ 12)
# endif
# ifndef __NR_time
#  define __NR_time (__NR_SYSCALL_BASE+ 13)
# endif
# ifndef __NR_mknod
#  define __NR_mknod (__NR_SYSCALL_BASE+ 14)
# endif
# ifndef __NR_chmod
#  define __NR_chmod (__NR_SYSCALL_BASE+ 15)
# endif
# ifndef __NR_lchown
#  define __NR_lchown (__NR_SYSCALL_BASE+ 16)
# endif
# ifndef __NR_lseek
#  define __NR_lseek (__NR_SYSCALL_BASE+ 19)
# endif
# ifndef __NR_getpid
#  define __NR_getpid (__NR_SYSCALL_BASE+ 20)
# endif
# ifndef __NR_mount
#  define __NR_mount (__NR_SYSCALL_BASE+ 21)
# endif
# ifndef __NR_umount
#  define __NR_umount (__NR_SYSCALL_BASE+ 22)
# endif
# ifndef __NR_setuid
#  define __NR_setuid (__NR_SYSCALL_BASE+ 23)
# endif
# ifndef __NR_getuid
#  define __NR_getuid (__NR_SYSCALL_BASE+ 24)
# endif
# ifndef __NR_stime
#  define __NR_stime (__NR_SYSCALL_BASE+ 25)
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace (__NR_SYSCALL_BASE+ 26)
# endif
# ifndef __NR_alarm
#  define __NR_alarm (__NR_SYSCALL_BASE+ 27)
# endif
# ifndef __NR_pause
#  define __NR_pause (__NR_SYSCALL_BASE+ 29)
# endif
# ifndef __NR_utime
#  define __NR_utime (__NR_SYSCALL_BASE+ 30)
# endif
# ifndef __NR_access
#  define __NR_access (__NR_SYSCALL_BASE+ 33)
# endif
# ifndef __NR_nice
#  define __NR_nice (__NR_SYSCALL_BASE+ 34)
# endif
# ifndef __NR_sync
#  define __NR_sync (__NR_SYSCALL_BASE+ 36)
# endif
# ifndef __NR_kill
#  define __NR_kill (__NR_SYSCALL_BASE+ 37)
# endif
# ifndef __NR_rename
#  define __NR_rename (__NR_SYSCALL_BASE+ 38)
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir (__NR_SYSCALL_BASE+ 39)
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir (__NR_SYSCALL_BASE+ 40)
# endif
# ifndef __NR_dup
#  define __NR_dup (__NR_SYSCALL_BASE+ 41)
# endif
# ifndef __NR_pipe
#  define __NR_pipe (__NR_SYSCALL_BASE+ 42)
# endif
# ifndef __NR_times
#  define __NR_times (__NR_SYSCALL_BASE+ 43)
# endif
# ifndef __NR_brk
#  define __NR_brk (__NR_SYSCALL_BASE+ 45)
# endif
# ifndef __NR_setgid
#  define __NR_setgid (__NR_SYSCALL_BASE+ 46)
# endif
# ifndef __NR_getgid
#  define __NR_getgid (__NR_SYSCALL_BASE+ 47)
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid (__NR_SYSCALL_BASE+ 49)
# endif
# ifndef __NR_getegid
#  define __NR_getegid (__NR_SYSCALL_BASE+ 50)
# endif
# ifndef __NR_acct
#  define __NR_acct (__NR_SYSCALL_BASE+ 51)
# endif
# ifndef __NR_umount2
#  define __NR_umount2 (__NR_SYSCALL_BASE+ 52)
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl (__NR_SYSCALL_BASE+ 54)
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl (__NR_SYSCALL_BASE+ 55)
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid (__NR_SYSCALL_BASE+ 57)
# endif
# ifndef __NR_umask
#  define __NR_umask (__NR_SYSCALL_BASE+ 60)
# endif
# ifndef __NR_chroot
#  define __NR_chroot (__NR_SYSCALL_BASE+ 61)
# endif
# ifndef __NR_ustat
#  define __NR_ustat (__NR_SYSCALL_BASE+ 62)
# endif
# ifndef __NR_dup2
#  define __NR_dup2 (__NR_SYSCALL_BASE+ 63)
# endif
# ifndef __NR_getppid
#  define __NR_getppid (__NR_SYSCALL_BASE+ 64)
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp (__NR_SYSCALL_BASE+ 65)
# endif
# ifndef __NR_setsid
#  define __NR_setsid (__NR_SYSCALL_BASE+ 66)
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction (__NR_SYSCALL_BASE+ 67)
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid (__NR_SYSCALL_BASE+ 70)
# endif
# ifndef __NR_setregid
#  define __NR_setregid (__NR_SYSCALL_BASE+ 71)
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend (__NR_SYSCALL_BASE+ 72)
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending (__NR_SYSCALL_BASE+ 73)
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname (__NR_SYSCALL_BASE+ 74)
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit (__NR_SYSCALL_BASE+ 75)
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit (__NR_SYSCALL_BASE+ 76)
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage (__NR_SYSCALL_BASE+ 77)
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday (__NR_SYSCALL_BASE+ 78)
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday (__NR_SYSCALL_BASE+ 79)
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups (__NR_SYSCALL_BASE+ 80)
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups (__NR_SYSCALL_BASE+ 81)
# endif
# ifndef __NR_select
#  define __NR_select (__NR_SYSCALL_BASE+ 82)
# endif
# ifndef __NR_symlink
#  define __NR_symlink (__NR_SYSCALL_BASE+ 83)
# endif
# ifndef __NR_readlink
#  define __NR_readlink (__NR_SYSCALL_BASE+ 85)
# endif
# ifndef __NR_uselib
#  define __NR_uselib (__NR_SYSCALL_BASE+ 86)
# endif
# ifndef __NR_swapon
#  define __NR_swapon (__NR_SYSCALL_BASE+ 87)
# endif
# ifndef __NR_reboot
#  define __NR_reboot (__NR_SYSCALL_BASE+ 88)
# endif
# ifndef __NR_readdir
#  define __NR_readdir (__NR_SYSCALL_BASE+ 89)
# endif
# ifndef __NR_mmap
#  define __NR_mmap (__NR_SYSCALL_BASE+ 90)
# endif
# ifndef __NR_munmap
#  define __NR_munmap (__NR_SYSCALL_BASE+ 91)
# endif
# ifndef __NR_truncate
#  define __NR_truncate (__NR_SYSCALL_BASE+ 92)
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate (__NR_SYSCALL_BASE+ 93)
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod (__NR_SYSCALL_BASE+ 94)
# endif
# ifndef __NR_fchown
#  define __NR_fchown (__NR_SYSCALL_BASE+ 95)
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority (__NR_SYSCALL_BASE+ 96)
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority (__NR_SYSCALL_BASE+ 97)
# endif
# ifndef __NR_statfs
#  define __NR_statfs (__NR_SYSCALL_BASE+ 99)
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs (__NR_SYSCALL_BASE+100)
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall (__NR_SYSCALL_BASE+102)
# endif
# ifndef __NR_syslog
#  define __NR_syslog (__NR_SYSCALL_BASE+103)
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer (__NR_SYSCALL_BASE+104)
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer (__NR_SYSCALL_BASE+105)
# endif
# ifndef __NR_stat
#  define __NR_stat (__NR_SYSCALL_BASE+106)
# endif
# ifndef __NR_lstat
#  define __NR_lstat (__NR_SYSCALL_BASE+107)
# endif
# ifndef __NR_fstat
#  define __NR_fstat (__NR_SYSCALL_BASE+108)
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup (__NR_SYSCALL_BASE+111)
# endif
# ifndef __NR_syscall
#  define __NR_syscall (__NR_SYSCALL_BASE+113)
# endif
# ifndef __NR_wait4
#  define __NR_wait4 (__NR_SYSCALL_BASE+114)
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff (__NR_SYSCALL_BASE+115)
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo (__NR_SYSCALL_BASE+116)
# endif
# ifndef __NR_ipc
#  define __NR_ipc (__NR_SYSCALL_BASE+117)
# endif
# ifndef __NR_fsync
#  define __NR_fsync (__NR_SYSCALL_BASE+118)
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn (__NR_SYSCALL_BASE+119)
# endif
# ifndef __NR_clone
#  define __NR_clone (__NR_SYSCALL_BASE+120)
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname (__NR_SYSCALL_BASE+121)
# endif
# ifndef __NR_uname
#  define __NR_uname (__NR_SYSCALL_BASE+122)
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex (__NR_SYSCALL_BASE+124)
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect (__NR_SYSCALL_BASE+125)
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask (__NR_SYSCALL_BASE+126)
# endif
# ifndef __NR_init_module
#  define __NR_init_module (__NR_SYSCALL_BASE+128)
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module (__NR_SYSCALL_BASE+129)
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl (__NR_SYSCALL_BASE+131)
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid (__NR_SYSCALL_BASE+132)
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir (__NR_SYSCALL_BASE+133)
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush (__NR_SYSCALL_BASE+134)
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs (__NR_SYSCALL_BASE+135)
# endif
# ifndef __NR_personality
#  define __NR_personality (__NR_SYSCALL_BASE+136)
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid (__NR_SYSCALL_BASE+138)
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid (__NR_SYSCALL_BASE+139)
# endif
# ifndef __NR_getdents
#  define __NR_getdents (__NR_SYSCALL_BASE+141)
# endif
# ifndef __NR_flock
#  define __NR_flock (__NR_SYSCALL_BASE+143)
# endif
# ifndef __NR_msync
#  define __NR_msync (__NR_SYSCALL_BASE+144)
# endif
# ifndef __NR_readv
#  define __NR_readv (__NR_SYSCALL_BASE+145)
# endif
# ifndef __NR_writev
#  define __NR_writev (__NR_SYSCALL_BASE+146)
# endif
# ifndef __NR_getsid
#  define __NR_getsid (__NR_SYSCALL_BASE+147)
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync (__NR_SYSCALL_BASE+148)
# endif
# ifndef __NR_mlock
#  define __NR_mlock (__NR_SYSCALL_BASE+150)
# endif
# ifndef __NR_munlock
#  define __NR_munlock (__NR_SYSCALL_BASE+151)
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall (__NR_SYSCALL_BASE+152)
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall (__NR_SYSCALL_BASE+153)
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam (__NR_SYSCALL_BASE+154)
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam (__NR_SYSCALL_BASE+155)
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler (__NR_SYSCALL_BASE+156)
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler (__NR_SYSCALL_BASE+157)
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield (__NR_SYSCALL_BASE+158)
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max (__NR_SYSCALL_BASE+159)
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min (__NR_SYSCALL_BASE+160)
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval (__NR_SYSCALL_BASE+161)
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep (__NR_SYSCALL_BASE+162)
# endif
# ifndef __NR_mremap
#  define __NR_mremap (__NR_SYSCALL_BASE+163)
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid (__NR_SYSCALL_BASE+164)
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid (__NR_SYSCALL_BASE+165)
# endif
# ifndef __NR_poll
#  define __NR_poll (__NR_SYSCALL_BASE+168)
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl (__NR_SYSCALL_BASE+169)
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid (__NR_SYSCALL_BASE+170)
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid (__NR_SYSCALL_BASE+171)
# endif
# ifndef __NR_prctl
#  define __NR_prctl (__NR_SYSCALL_BASE+172)
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn (__NR_SYSCALL_BASE+173)
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction (__NR_SYSCALL_BASE+174)
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask (__NR_SYSCALL_BASE+175)
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending (__NR_SYSCALL_BASE+176)
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait (__NR_SYSCALL_BASE+177)
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo (__NR_SYSCALL_BASE+178)
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend (__NR_SYSCALL_BASE+179)
# endif
# ifndef __NR_pread64
#  define __NR_pread64 (__NR_SYSCALL_BASE+180)
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 (__NR_SYSCALL_BASE+181)
# endif
# ifndef __NR_chown
#  define __NR_chown (__NR_SYSCALL_BASE+182)
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd (__NR_SYSCALL_BASE+183)
# endif
# ifndef __NR_capget
#  define __NR_capget (__NR_SYSCALL_BASE+184)
# endif
# ifndef __NR_capset
#  define __NR_capset (__NR_SYSCALL_BASE+185)
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack (__NR_SYSCALL_BASE+186)
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile (__NR_SYSCALL_BASE+187)
# endif
# ifndef __NR_vfork
#  define __NR_vfork (__NR_SYSCALL_BASE+190)
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit (__NR_SYSCALL_BASE+191)
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 (__NR_SYSCALL_BASE+192)
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 (__NR_SYSCALL_BASE+193)
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 (__NR_SYSCALL_BASE+194)
# endif
# ifndef __NR_stat64
#  define __NR_stat64 (__NR_SYSCALL_BASE+195)
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 (__NR_SYSCALL_BASE+196)
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 (__NR_SYSCALL_BASE+197)
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 (__NR_SYSCALL_BASE+198)
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 (__NR_SYSCALL_BASE+199)
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 (__NR_SYSCALL_BASE+200)
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 (__NR_SYSCALL_BASE+201)
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 (__NR_SYSCALL_BASE+202)
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 (__NR_SYSCALL_BASE+203)
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 (__NR_SYSCALL_BASE+204)
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 (__NR_SYSCALL_BASE+205)
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 (__NR_SYSCALL_BASE+206)
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 (__NR_SYSCALL_BASE+207)
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 (__NR_SYSCALL_BASE+208)
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 (__NR_SYSCALL_BASE+209)
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 (__NR_SYSCALL_BASE+210)
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 (__NR_SYSCALL_BASE+211)
# endif
# ifndef __NR_chown32
#  define __NR_chown32 (__NR_SYSCALL_BASE+212)
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 (__NR_SYSCALL_BASE+213)
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 (__NR_SYSCALL_BASE+214)
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 (__NR_SYSCALL_BASE+215)
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 (__NR_SYSCALL_BASE+216)
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 (__NR_SYSCALL_BASE+217)
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root (__NR_SYSCALL_BASE+218)
# endif
# ifndef __NR_mincore
#  define __NR_mincore (__NR_SYSCALL_BASE+219)
# endif
# ifndef __NR_madvise
#  define __NR_madvise (__NR_SYSCALL_BASE+220)
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 (__NR_SYSCALL_BASE+221)
# endif
# ifndef __NR_gettid
#  define __NR_gettid (__NR_SYSCALL_BASE+224)
# endif
# ifndef __NR_readahead
#  define __NR_readahead (__NR_SYSCALL_BASE+225)
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr (__NR_SYSCALL_BASE+226)
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr (__NR_SYSCALL_BASE+227)
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr (__NR_SYSCALL_BASE+228)
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr (__NR_SYSCALL_BASE+229)
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr (__NR_SYSCALL_BASE+230)
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr (__NR_SYSCALL_BASE+231)
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr (__NR_SYSCALL_BASE+232)
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr (__NR_SYSCALL_BASE+233)
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr (__NR_SYSCALL_BASE+234)
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr (__NR_SYSCALL_BASE+235)
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr (__NR_SYSCALL_BASE+236)
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr (__NR_SYSCALL_BASE+237)
# endif
# ifndef __NR_tkill
#  define __NR_tkill (__NR_SYSCALL_BASE+238)
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 (__NR_SYSCALL_BASE+239)
# endif
# ifndef __NR_futex
#  define __NR_futex (__NR_SYSCALL_BASE+240)
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity (__NR_SYSCALL_BASE+241)
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity (__NR_SYSCALL_BASE+242)
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup (__NR_SYSCALL_BASE+243)
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy (__NR_SYSCALL_BASE+244)
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents (__NR_SYSCALL_BASE+245)
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit (__NR_SYSCALL_BASE+246)
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel (__NR_SYSCALL_BASE+247)
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group (__NR_SYSCALL_BASE+248)
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie (__NR_SYSCALL_BASE+249)
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create (__NR_SYSCALL_BASE+250)
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl (__NR_SYSCALL_BASE+251)
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait (__NR_SYSCALL_BASE+252)
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages (__NR_SYSCALL_BASE+253)
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address (__NR_SYSCALL_BASE+256)
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create (__NR_SYSCALL_BASE+257)
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime (__NR_SYSCALL_BASE+258)
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime (__NR_SYSCALL_BASE+259)
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun (__NR_SYSCALL_BASE+260)
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete (__NR_SYSCALL_BASE+261)
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime (__NR_SYSCALL_BASE+262)
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime (__NR_SYSCALL_BASE+263)
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres (__NR_SYSCALL_BASE+264)
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep (__NR_SYSCALL_BASE+265)
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 (__NR_SYSCALL_BASE+266)
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 (__NR_SYSCALL_BASE+267)
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill (__NR_SYSCALL_BASE+268)
# endif
# ifndef __NR_utimes
#  define __NR_utimes (__NR_SYSCALL_BASE+269)
# endif
# ifndef __NR_arm_fadvise64_64
#  define __NR_arm_fadvise64_64 (__NR_SYSCALL_BASE+270)
# endif
# ifndef __NR_pciconfig_iobase
#  define __NR_pciconfig_iobase (__NR_SYSCALL_BASE+271)
# endif
# ifndef __NR_pciconfig_read
#  define __NR_pciconfig_read (__NR_SYSCALL_BASE+272)
# endif
# ifndef __NR_pciconfig_write
#  define __NR_pciconfig_write (__NR_SYSCALL_BASE+273)
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open (__NR_SYSCALL_BASE+274)
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink (__NR_SYSCALL_BASE+275)
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend (__NR_SYSCALL_BASE+276)
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive (__NR_SYSCALL_BASE+277)
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify (__NR_SYSCALL_BASE+278)
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr (__NR_SYSCALL_BASE+279)
# endif
# ifndef __NR_waitid
#  define __NR_waitid (__NR_SYSCALL_BASE+280)
# endif
# ifndef __NR_socket
#  define __NR_socket (__NR_SYSCALL_BASE+281)
# endif
# ifndef __NR_bind
#  define __NR_bind (__NR_SYSCALL_BASE+282)
# endif
# ifndef __NR_connect
#  define __NR_connect (__NR_SYSCALL_BASE+283)
# endif
# ifndef __NR_listen
#  define __NR_listen (__NR_SYSCALL_BASE+284)
# endif
# ifndef __NR_accept
#  define __NR_accept (__NR_SYSCALL_BASE+285)
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname (__NR_SYSCALL_BASE+286)
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername (__NR_SYSCALL_BASE+287)
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair (__NR_SYSCALL_BASE+288)
# endif
# ifndef __NR_send
#  define __NR_send (__NR_SYSCALL_BASE+289)
# endif
# ifndef __NR_sendto
#  define __NR_sendto (__NR_SYSCALL_BASE+290)
# endif
# ifndef __NR_recv
#  define __NR_recv (__NR_SYSCALL_BASE+291)
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom (__NR_SYSCALL_BASE+292)
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown (__NR_SYSCALL_BASE+293)
# endif
# ifndef __NR_setsockopt
#  define __NR_setsockopt (__NR_SYSCALL_BASE+294)
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt (__NR_SYSCALL_BASE+295)
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg (__NR_SYSCALL_BASE+296)
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg (__NR_SYSCALL_BASE+297)
# endif
# ifndef __NR_semop
#  define __NR_semop (__NR_SYSCALL_BASE+298)
# endif
# ifndef __NR_semget
#  define __NR_semget (__NR_SYSCALL_BASE+299)
# endif
# ifndef __NR_semctl
#  define __NR_semctl (__NR_SYSCALL_BASE+300)
# endif
# ifndef __NR_msgsnd
#  define __NR_msgsnd (__NR_SYSCALL_BASE+301)
# endif
# ifndef __NR_msgrcv
#  define __NR_msgrcv (__NR_SYSCALL_BASE+302)
# endif
# ifndef __NR_msgget
#  define __NR_msgget (__NR_SYSCALL_BASE+303)
# endif
# ifndef __NR_msgctl
#  define __NR_msgctl (__NR_SYSCALL_BASE+304)
# endif
# ifndef __NR_shmat
#  define __NR_shmat (__NR_SYSCALL_BASE+305)
# endif
# ifndef __NR_shmdt
#  define __NR_shmdt (__NR_SYSCALL_BASE+306)
# endif
# ifndef __NR_shmget
#  define __NR_shmget (__NR_SYSCALL_BASE+307)
# endif
# ifndef __NR_shmctl
#  define __NR_shmctl (__NR_SYSCALL_BASE+308)
# endif
# ifndef __NR_add_key
#  define __NR_add_key (__NR_SYSCALL_BASE+309)
# endif
# ifndef __NR_request_key
#  define __NR_request_key (__NR_SYSCALL_BASE+310)
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl (__NR_SYSCALL_BASE+311)
# endif
# ifndef __NR_semtimedop
#  define __NR_semtimedop (__NR_SYSCALL_BASE+312)
# endif
# ifndef __NR_vserver
#  define __NR_vserver (__NR_SYSCALL_BASE+313)
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set (__NR_SYSCALL_BASE+314)
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get (__NR_SYSCALL_BASE+315)
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init (__NR_SYSCALL_BASE+316)
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch (__NR_SYSCALL_BASE+317)
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch (__NR_SYSCALL_BASE+318)
# endif
# ifndef __NR_mbind
#  define __NR_mbind (__NR_SYSCALL_BASE+319)
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy (__NR_SYSCALL_BASE+320)
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy (__NR_SYSCALL_BASE+321)
# endif
# ifndef __NR_openat
#  define __NR_openat (__NR_SYSCALL_BASE+322)
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat (__NR_SYSCALL_BASE+323)
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat (__NR_SYSCALL_BASE+324)
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat (__NR_SYSCALL_BASE+325)
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat (__NR_SYSCALL_BASE+326)
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 (__NR_SYSCALL_BASE+327)
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat (__NR_SYSCALL_BASE+328)
# endif
# ifndef __NR_renameat
#  define __NR_renameat (__NR_SYSCALL_BASE+329)
# endif
# ifndef __NR_linkat
#  define __NR_linkat (__NR_SYSCALL_BASE+330)
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat (__NR_SYSCALL_BASE+331)
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat (__NR_SYSCALL_BASE+332)
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat (__NR_SYSCALL_BASE+333)
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat (__NR_SYSCALL_BASE+334)
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 (__NR_SYSCALL_BASE+335)
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll (__NR_SYSCALL_BASE+336)
# endif
# ifndef __NR_unshare
#  define __NR_unshare (__NR_SYSCALL_BASE+337)
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list (__NR_SYSCALL_BASE+338)
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list (__NR_SYSCALL_BASE+339)
# endif
# ifndef __NR_splice
#  define __NR_splice (__NR_SYSCALL_BASE+340)
# endif
# ifndef __NR_arm_sync_file_range
#  define __NR_arm_sync_file_range (__NR_SYSCALL_BASE+341)
# endif
# ifndef __NR_sync_file_range2
#  define __NR_sync_file_range2 __NR_arm_sync_file_range
# endif
# ifndef __NR_tee
#  define __NR_tee (__NR_SYSCALL_BASE+342)
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice (__NR_SYSCALL_BASE+343)
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages (__NR_SYSCALL_BASE+344)
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu (__NR_SYSCALL_BASE+345)
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait (__NR_SYSCALL_BASE+346)
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load (__NR_SYSCALL_BASE+347)
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat (__NR_SYSCALL_BASE+348)
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd (__NR_SYSCALL_BASE+349)
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create (__NR_SYSCALL_BASE+350)
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd (__NR_SYSCALL_BASE+351)
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate (__NR_SYSCALL_BASE+352)
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime (__NR_SYSCALL_BASE+353)
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime (__NR_SYSCALL_BASE+354)
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 (__NR_SYSCALL_BASE+355)
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 (__NR_SYSCALL_BASE+356)
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 (__NR_SYSCALL_BASE+357)
# endif
# ifndef __NR_dup3
#  define __NR_dup3 (__NR_SYSCALL_BASE+358)
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 (__NR_SYSCALL_BASE+359)
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 (__NR_SYSCALL_BASE+360)
# endif
# ifndef __NR_preadv
#  define __NR_preadv (__NR_SYSCALL_BASE+361)
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev (__NR_SYSCALL_BASE+362)
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo (__NR_SYSCALL_BASE+363)
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open (__NR_SYSCALL_BASE+364)
# endif
#endif


#ifdef __hppa__
# ifndef __NR_openat
#  define __NR_openat 275
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat (__NR_openat + 1)
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat (__NR_openat + 2)
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat (__NR_openat + 3)
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat (__NR_openat + 4)
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat (__NR_openat + 5)
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 (__NR_openat + 5)
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat (__NR_openat + 6)
# endif
# ifndef __NR_renameat
#  define __NR_renameat (__NR_openat + 7)
# endif
# ifndef __NR_linkat
#  define __NR_linkat (__NR_openat + 8)
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat (__NR_openat + 9)
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat (__NR_openat + 10)
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat (__NR_openat + 11)
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat (__NR_openat + 12)
# endif
# ifndef __NR_splice
#  define __NR_splice 291
# endif
# ifndef __NR_tee
#  define __NR_tee 293
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 294
# endif
#endif


#ifdef __i386__
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 0
# endif
# ifndef __NR_exit
#  define __NR_exit 1
# endif
# ifndef __NR_fork
#  define __NR_fork 2
# endif
# ifndef __NR_read
#  define __NR_read 3
# endif
# ifndef __NR_write
#  define __NR_write 4
# endif
# ifndef __NR_open
#  define __NR_open 5
# endif
# ifndef __NR_close
#  define __NR_close 6
# endif
# ifndef __NR_waitpid
#  define __NR_waitpid 7
# endif
# ifndef __NR_creat
#  define __NR_creat 8
# endif
# ifndef __NR_link
#  define __NR_link 9
# endif
# ifndef __NR_unlink
#  define __NR_unlink 10
# endif
# ifndef __NR_execve
#  define __NR_execve 11
# endif
# ifndef __NR_chdir
#  define __NR_chdir 12
# endif
# ifndef __NR_time
#  define __NR_time 13
# endif
# ifndef __NR_mknod
#  define __NR_mknod 14
# endif
# ifndef __NR_chmod
#  define __NR_chmod 15
# endif
# ifndef __NR_lchown
#  define __NR_lchown 16
# endif
# ifndef __NR_break
#  define __NR_break 17
# endif
# ifndef __NR_oldstat
#  define __NR_oldstat 18
# endif
# ifndef __NR_lseek
#  define __NR_lseek 19
# endif
# ifndef __NR_getpid
#  define __NR_getpid 20
# endif
# ifndef __NR_mount
#  define __NR_mount 21
# endif
# ifndef __NR_umount
#  define __NR_umount 22
# endif
# ifndef __NR_setuid
#  define __NR_setuid 23
# endif
# ifndef __NR_getuid
#  define __NR_getuid 24
# endif
# ifndef __NR_stime
#  define __NR_stime 25
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 26
# endif
# ifndef __NR_alarm
#  define __NR_alarm 27
# endif
# ifndef __NR_oldfstat
#  define __NR_oldfstat 28
# endif
# ifndef __NR_pause
#  define __NR_pause 29
# endif
# ifndef __NR_utime
#  define __NR_utime 30
# endif
# ifndef __NR_stty
#  define __NR_stty 31
# endif
# ifndef __NR_gtty
#  define __NR_gtty 32
# endif
# ifndef __NR_access
#  define __NR_access 33
# endif
# ifndef __NR_nice
#  define __NR_nice 34
# endif
# ifndef __NR_ftime
#  define __NR_ftime 35
# endif
# ifndef __NR_sync
#  define __NR_sync 36
# endif
# ifndef __NR_kill
#  define __NR_kill 37
# endif
# ifndef __NR_rename
#  define __NR_rename 38
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 39
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 40
# endif
# ifndef __NR_dup
#  define __NR_dup 41
# endif
# ifndef __NR_pipe
#  define __NR_pipe 42
# endif
# ifndef __NR_times
#  define __NR_times 43
# endif
# ifndef __NR_prof
#  define __NR_prof 44
# endif
# ifndef __NR_brk
#  define __NR_brk 45
# endif
# ifndef __NR_setgid
#  define __NR_setgid 46
# endif
# ifndef __NR_getgid
#  define __NR_getgid 47
# endif
# ifndef __NR_signal
#  define __NR_signal 48
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 49
# endif
# ifndef __NR_getegid
#  define __NR_getegid 50
# endif
# ifndef __NR_acct
#  define __NR_acct 51
# endif
# ifndef __NR_umount2
#  define __NR_umount2 52
# endif
# ifndef __NR_lock
#  define __NR_lock 53
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 54
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 55
# endif
# ifndef __NR_mpx
#  define __NR_mpx 56
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 57
# endif
# ifndef __NR_ulimit
#  define __NR_ulimit 58
# endif
# ifndef __NR_oldolduname
#  define __NR_oldolduname 59
# endif
# ifndef __NR_umask
#  define __NR_umask 60
# endif
# ifndef __NR_chroot
#  define __NR_chroot 61
# endif
# ifndef __NR_ustat
#  define __NR_ustat 62
# endif
# ifndef __NR_dup2
#  define __NR_dup2 63
# endif
# ifndef __NR_getppid
#  define __NR_getppid 64
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 65
# endif
# ifndef __NR_setsid
#  define __NR_setsid 66
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 67
# endif
# ifndef __NR_sgetmask
#  define __NR_sgetmask 68
# endif
# ifndef __NR_ssetmask
#  define __NR_ssetmask 69
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 70
# endif
# ifndef __NR_setregid
#  define __NR_setregid 71
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 72
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 73
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 74
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 75
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 76
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 77
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 78
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 79
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 80
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 81
# endif
# ifndef __NR_select
#  define __NR_select 82
# endif
# ifndef __NR_symlink
#  define __NR_symlink 83
# endif
# ifndef __NR_oldlstat
#  define __NR_oldlstat 84
# endif
# ifndef __NR_readlink
#  define __NR_readlink 85
# endif
# ifndef __NR_uselib
#  define __NR_uselib 86
# endif
# ifndef __NR_swapon
#  define __NR_swapon 87
# endif
# ifndef __NR_reboot
#  define __NR_reboot 88
# endif
# ifndef __NR_readdir
#  define __NR_readdir 89
# endif
# ifndef __NR_mmap
#  define __NR_mmap 90
# endif
# ifndef __NR_munmap
#  define __NR_munmap 91
# endif
# ifndef __NR_truncate
#  define __NR_truncate 92
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 93
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 94
# endif
# ifndef __NR_fchown
#  define __NR_fchown 95
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 96
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 97
# endif
# ifndef __NR_profil
#  define __NR_profil 98
# endif
# ifndef __NR_statfs
#  define __NR_statfs 99
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 100
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 101
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 102
# endif
# ifndef __NR_syslog
#  define __NR_syslog 103
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 104
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 105
# endif
# ifndef __NR_stat
#  define __NR_stat 106
# endif
# ifndef __NR_lstat
#  define __NR_lstat 107
# endif
# ifndef __NR_fstat
#  define __NR_fstat 108
# endif
# ifndef __NR_olduname
#  define __NR_olduname 109
# endif
# ifndef __NR_iopl
#  define __NR_iopl 110
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 111
# endif
# ifndef __NR_idle
#  define __NR_idle 112
# endif
# ifndef __NR_vm86old
#  define __NR_vm86old 113
# endif
# ifndef __NR_wait4
#  define __NR_wait4 114
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 115
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 116
# endif
# ifndef __NR_ipc
#  define __NR_ipc 117
# endif
# ifndef __NR_fsync
#  define __NR_fsync 118
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 119
# endif
# ifndef __NR_clone
#  define __NR_clone 120
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 121
# endif
# ifndef __NR_uname
#  define __NR_uname 122
# endif
# ifndef __NR_modify_ldt
#  define __NR_modify_ldt 123
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 124
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 125
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 126
# endif
# ifndef __NR_create_module
#  define __NR_create_module 127
# endif
# ifndef __NR_init_module
#  define __NR_init_module 128
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 129
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 130
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 131
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 132
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 133
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 134
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 135
# endif
# ifndef __NR_personality
#  define __NR_personality 136
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 137
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 138
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 139
# endif
# ifndef __NR_getdents
#  define __NR_getdents 141
# endif
# ifndef __NR_flock
#  define __NR_flock 143
# endif
# ifndef __NR_msync
#  define __NR_msync 144
# endif
# ifndef __NR_readv
#  define __NR_readv 145
# endif
# ifndef __NR_writev
#  define __NR_writev 146
# endif
# ifndef __NR_getsid
#  define __NR_getsid 147
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 148
# endif
# ifndef __NR_mlock
#  define __NR_mlock 150
# endif
# ifndef __NR_munlock
#  define __NR_munlock 151
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 152
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 153
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 154
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 155
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 156
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 157
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 158
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 159
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 160
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 161
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 162
# endif
# ifndef __NR_mremap
#  define __NR_mremap 163
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 164
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 165
# endif
# ifndef __NR_vm86
#  define __NR_vm86 166
# endif
# ifndef __NR_query_module
#  define __NR_query_module 167
# endif
# ifndef __NR_poll
#  define __NR_poll 168
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 169
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 170
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 171
# endif
# ifndef __NR_prctl
#  define __NR_prctl 172
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 173
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 174
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 175
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 176
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 177
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 178
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 179
# endif
# ifndef __NR_pread64
#  define __NR_pread64 180
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 181
# endif
# ifndef __NR_chown
#  define __NR_chown 182
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 183
# endif
# ifndef __NR_capget
#  define __NR_capget 184
# endif
# ifndef __NR_capset
#  define __NR_capset 185
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 186
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 187
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 188
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 189
# endif
# ifndef __NR_vfork
#  define __NR_vfork 190
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit 191
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 192
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 193
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 194
# endif
# ifndef __NR_stat64
#  define __NR_stat64 195
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 196
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 197
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 198
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 199
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 200
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 201
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 202
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 203
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 204
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 205
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 206
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 207
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 208
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 209
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 210
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 211
# endif
# ifndef __NR_chown32
#  define __NR_chown32 212
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 213
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 214
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 215
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 216
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 217
# endif
# ifndef __NR_mincore
#  define __NR_mincore 218
# endif
# ifndef __NR_madvise
#  define __NR_madvise 219
# endif
# ifndef __NR_madvise1
#  define __NR_madvise1 219
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 220
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 221
# endif
# ifndef __NR_gettid
#  define __NR_gettid 224
# endif
# ifndef __NR_readahead
#  define __NR_readahead 225
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 226
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 227
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 228
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 229
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 230
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 231
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 232
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 233
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 234
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 235
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 236
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 237
# endif
# ifndef __NR_tkill
#  define __NR_tkill 238
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 239
# endif
# ifndef __NR_futex
#  define __NR_futex 240
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 241
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 242
# endif
# ifndef __NR_set_thread_area
#  define __NR_set_thread_area 243
# endif
# ifndef __NR_get_thread_area
#  define __NR_get_thread_area 244
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 245
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 246
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 247
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 248
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 249
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 250
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 252
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 253
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 254
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 255
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 256
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 257
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 258
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 259
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime (__NR_timer_create+1)
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime (__NR_timer_create+2)
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun (__NR_timer_create+3)
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete (__NR_timer_create+4)
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime (__NR_timer_create+5)
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime (__NR_timer_create+6)
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres (__NR_timer_create+7)
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep (__NR_timer_create+8)
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 268
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 269
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 270
# endif
# ifndef __NR_utimes
#  define __NR_utimes 271
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 272
# endif
# ifndef __NR_vserver
#  define __NR_vserver 273
# endif
# ifndef __NR_mbind
#  define __NR_mbind 274
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 275
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 276
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 277
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink (__NR_mq_open+1)
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend (__NR_mq_open+2)
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive (__NR_mq_open+3)
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify (__NR_mq_open+4)
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr (__NR_mq_open+5)
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 283
# endif
# ifndef __NR_waitid
#  define __NR_waitid 284
# endif
# ifndef __NR_add_key
#  define __NR_add_key 286
# endif
# ifndef __NR_request_key
#  define __NR_request_key 287
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 288
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 289
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 290
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 291
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 292
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 293
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 294
# endif
# ifndef __NR_openat
#  define __NR_openat 295
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 296
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 297
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 298
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 299
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 300
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 301
# endif
# ifndef __NR_renameat
#  define __NR_renameat 302
# endif
# ifndef __NR_linkat
#  define __NR_linkat 303
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 304
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 305
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 306
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 307
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 308
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 309
# endif
# ifndef __NR_unshare
#  define __NR_unshare 310
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 311
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 312
# endif
# ifndef __NR_splice
#  define __NR_splice 313
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 314
# endif
# ifndef __NR_tee
#  define __NR_tee 315
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 316
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 317
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 318
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 319
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 320
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 321
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 322
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 323
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 324
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 325
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 326
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 327
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 328
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 329
# endif
# ifndef __NR_dup3
#  define __NR_dup3 330
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 331
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 332
# endif
# ifndef __NR_preadv
#  define __NR_preadv 333
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 334
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 335
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 336
# endif
#endif


#ifdef __ia64__
# ifndef __NR_ni_syscall
#  define __NR_ni_syscall 1024
# endif
# ifndef __NR_exit
#  define __NR_exit 1025
# endif
# ifndef __NR_read
#  define __NR_read 1026
# endif
# ifndef __NR_write
#  define __NR_write 1027
# endif
# ifndef __NR_open
#  define __NR_open 1028
# endif
# ifndef __NR_close
#  define __NR_close 1029
# endif
# ifndef __NR_creat
#  define __NR_creat 1030
# endif
# ifndef __NR_link
#  define __NR_link 1031
# endif
# ifndef __NR_unlink
#  define __NR_unlink 1032
# endif
# ifndef __NR_execve
#  define __NR_execve 1033
# endif
# ifndef __NR_chdir
#  define __NR_chdir 1034
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 1035
# endif
# ifndef __NR_utimes
#  define __NR_utimes 1036
# endif
# ifndef __NR_mknod
#  define __NR_mknod 1037
# endif
# ifndef __NR_chmod
#  define __NR_chmod 1038
# endif
# ifndef __NR_chown
#  define __NR_chown 1039
# endif
# ifndef __NR_lseek
#  define __NR_lseek 1040
# endif
# ifndef __NR_getpid
#  define __NR_getpid 1041
# endif
# ifndef __NR_getppid
#  define __NR_getppid 1042
# endif
# ifndef __NR_mount
#  define __NR_mount 1043
# endif
# ifndef __NR_umount
#  define __NR_umount 1044
# endif
# ifndef __NR_setuid
#  define __NR_setuid 1045
# endif
# ifndef __NR_getuid
#  define __NR_getuid 1046
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 1047
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 1048
# endif
# ifndef __NR_access
#  define __NR_access 1049
# endif
# ifndef __NR_sync
#  define __NR_sync 1050
# endif
# ifndef __NR_fsync
#  define __NR_fsync 1051
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 1052
# endif
# ifndef __NR_kill
#  define __NR_kill 1053
# endif
# ifndef __NR_rename
#  define __NR_rename 1054
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 1055
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 1056
# endif
# ifndef __NR_dup
#  define __NR_dup 1057
# endif
# ifndef __NR_pipe
#  define __NR_pipe 1058
# endif
# ifndef __NR_times
#  define __NR_times 1059
# endif
# ifndef __NR_brk
#  define __NR_brk 1060
# endif
# ifndef __NR_setgid
#  define __NR_setgid 1061
# endif
# ifndef __NR_getgid
#  define __NR_getgid 1062
# endif
# ifndef __NR_getegid
#  define __NR_getegid 1063
# endif
# ifndef __NR_acct
#  define __NR_acct 1064
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 1065
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 1066
# endif
# ifndef __NR_umask
#  define __NR_umask 1067
# endif
# ifndef __NR_chroot
#  define __NR_chroot 1068
# endif
# ifndef __NR_ustat
#  define __NR_ustat 1069
# endif
# ifndef __NR_dup2
#  define __NR_dup2 1070
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 1071
# endif
# ifndef __NR_setregid
#  define __NR_setregid 1072
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 1073
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 1074
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 1075
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 1076
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 1077
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 1078
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 1079
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 1080
# endif
# ifndef __NR_setsid
#  define __NR_setsid 1081
# endif
# ifndef __NR_getsid
#  define __NR_getsid 1082
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 1083
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 1084
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 1085
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 1086
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 1087
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 1088
# endif
# ifndef __NR_select
#  define __NR_select 1089
# endif
# ifndef __NR_poll
#  define __NR_poll 1090
# endif
# ifndef __NR_symlink
#  define __NR_symlink 1091
# endif
# ifndef __NR_readlink
#  define __NR_readlink 1092
# endif
# ifndef __NR_uselib
#  define __NR_uselib 1093
# endif
# ifndef __NR_swapon
#  define __NR_swapon 1094
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 1095
# endif
# ifndef __NR_reboot
#  define __NR_reboot 1096
# endif
# ifndef __NR_truncate
#  define __NR_truncate 1097
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 1098
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 1099
# endif
# ifndef __NR_fchown
#  define __NR_fchown 1100
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 1101
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 1102
# endif
# ifndef __NR_statfs
#  define __NR_statfs 1103
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 1104
# endif
# ifndef __NR_gettid
#  define __NR_gettid 1105
# endif
# ifndef __NR_semget
#  define __NR_semget 1106
# endif
# ifndef __NR_semop
#  define __NR_semop 1107
# endif
# ifndef __NR_semctl
#  define __NR_semctl 1108
# endif
# ifndef __NR_msgget
#  define __NR_msgget 1109
# endif
# ifndef __NR_msgsnd
#  define __NR_msgsnd 1110
# endif
# ifndef __NR_msgrcv
#  define __NR_msgrcv 1111
# endif
# ifndef __NR_msgctl
#  define __NR_msgctl 1112
# endif
# ifndef __NR_shmget
#  define __NR_shmget 1113
# endif
# ifndef __NR_shmat
#  define __NR_shmat 1114
# endif
# ifndef __NR_shmdt
#  define __NR_shmdt 1115
# endif
# ifndef __NR_shmctl
#  define __NR_shmctl 1116
# endif
# ifndef __NR_syslog
#  define __NR_syslog 1117
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 1118
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 1119
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 1123
# endif
# ifndef __NR_lchown
#  define __NR_lchown 1124
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 1125
# endif
# ifndef __NR_wait4
#  define __NR_wait4 1126
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 1127
# endif
# ifndef __NR_clone
#  define __NR_clone 1128
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 1129
# endif
# ifndef __NR_uname
#  define __NR_uname 1130
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 1131
# endif
# ifndef __NR_init_module
#  define __NR_init_module 1133
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 1134
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 1137
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 1138
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 1139
# endif
# ifndef __NR_personality
#  define __NR_personality 1140
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 1141
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 1142
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 1143
# endif
# ifndef __NR_getdents
#  define __NR_getdents 1144
# endif
# ifndef __NR_flock
#  define __NR_flock 1145
# endif
# ifndef __NR_readv
#  define __NR_readv 1146
# endif
# ifndef __NR_writev
#  define __NR_writev 1147
# endif
# ifndef __NR_pread64
#  define __NR_pread64 1148
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 1149
# endif
# ifndef __NR_mmap
#  define __NR_mmap 1151
# endif
# ifndef __NR_munmap
#  define __NR_munmap 1152
# endif
# ifndef __NR_mlock
#  define __NR_mlock 1153
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 1154
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 1155
# endif
# ifndef __NR_mremap
#  define __NR_mremap 1156
# endif
# ifndef __NR_msync
#  define __NR_msync 1157
# endif
# ifndef __NR_munlock
#  define __NR_munlock 1158
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 1159
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 1160
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 1161
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 1162
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 1163
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 1164
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 1165
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 1166
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 1167
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 1168
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 1169
# endif
# ifndef __NR_prctl
#  define __NR_prctl 1170
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 1172
# endif
# ifndef __NR_pciconfig_read
#  define __NR_pciconfig_read 1173
# endif
# ifndef __NR_pciconfig_write
#  define __NR_pciconfig_write 1174
# endif
# ifndef __NR_perfmonctl
#  define __NR_perfmonctl 1175
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 1176
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 1177
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 1178
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 1179
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 1180
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 1181
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 1182
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 1183
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 1184
# endif
# ifndef __NR_capget
#  define __NR_capget 1185
# endif
# ifndef __NR_capset
#  define __NR_capset 1186
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 1187
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 1188
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 1189
# endif
# ifndef __NR_socket
#  define __NR_socket 1190
# endif
# ifndef __NR_bind
#  define __NR_bind 1191
# endif
# ifndef __NR_connect
#  define __NR_connect 1192
# endif
# ifndef __NR_listen
#  define __NR_listen 1193
# endif
# ifndef __NR_accept
#  define __NR_accept 1194
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname 1195
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername 1196
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair 1197
# endif
# ifndef __NR_send
#  define __NR_send 1198
# endif
# ifndef __NR_sendto
#  define __NR_sendto 1199
# endif
# ifndef __NR_recv
#  define __NR_recv 1200
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom 1201
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown 1202
# endif
# ifndef __NR_setsockopt
#  define __NR_setsockopt 1203
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt 1204
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg 1205
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg 1206
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 1207
# endif
# ifndef __NR_mincore
#  define __NR_mincore 1208
# endif
# ifndef __NR_madvise
#  define __NR_madvise 1209
# endif
# ifndef __NR_stat
#  define __NR_stat 1210
# endif
# ifndef __NR_lstat
#  define __NR_lstat 1211
# endif
# ifndef __NR_fstat
#  define __NR_fstat 1212
# endif
# ifndef __NR_clone2
#  define __NR_clone2 1213
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 1214
# endif
# ifndef __NR_getunwind
#  define __NR_getunwind 1215
# endif
# ifndef __NR_readahead
#  define __NR_readahead 1216
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 1217
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 1218
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 1219
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 1220
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 1221
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 1222
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 1223
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 1224
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 1225
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 1226
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 1227
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 1228
# endif
# ifndef __NR_tkill
#  define __NR_tkill 1229
# endif
# ifndef __NR_futex
#  define __NR_futex 1230
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 1231
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 1232
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 1233
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 1234
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 1235
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 1236
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 1237
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 1238
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 1239
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 1240
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 1241
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 1242
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 1243
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 1244
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 1245
# endif
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 1246
# endif
# ifndef __NR_semtimedop
#  define __NR_semtimedop 1247
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 1248
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 1249
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime 1250
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun 1251
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 1252
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 1253
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 1254
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres 1255
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep 1256
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 1257
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 1258
# endif
# ifndef __NR_mbind
#  define __NR_mbind 1259
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 1260
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 1261
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 1262
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 1263
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 1264
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 1265
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 1266
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 1267
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 1268
# endif
# ifndef __NR_vserver
#  define __NR_vserver 1269
# endif
# ifndef __NR_waitid
#  define __NR_waitid 1270
# endif
# ifndef __NR_add_key
#  define __NR_add_key 1271
# endif
# ifndef __NR_request_key
#  define __NR_request_key 1272
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 1273
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 1274
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 1275
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 1276
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 1277
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 1278
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 1279
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 1280
# endif
# ifndef __NR_openat
#  define __NR_openat 1281
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 1282
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 1283
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 1284
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 1285
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 1286
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 1287
# endif
# ifndef __NR_renameat
#  define __NR_renameat 1288
# endif
# ifndef __NR_linkat
#  define __NR_linkat 1289
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 1290
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 1291
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 1292
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 1293
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 1294
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 1295
# endif
# ifndef __NR_unshare
#  define __NR_unshare 1296
# endif
# ifndef __NR_splice
#  define __NR_splice 1297
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 1298
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 1299
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 1300
# endif
# ifndef __NR_tee
#  define __NR_tee 1301
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 1302
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 1303
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 1304
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 1305
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 1306
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 1307
# endif
# ifndef __NR_timerfd
#  define __NR_timerfd 1308
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 1309
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 1310
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 1311
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 1312
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 1313
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 1314
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 1315
# endif
# ifndef __NR_dup3
#  define __NR_dup3 1316
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 1317
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 1318
# endif
# ifndef __NR_preadv
#  define __NR_preadv 1319
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 1320
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 1321
# endif
#endif


#ifdef __powerpc64__
# ifndef __NR_timer_create
#  define __NR_timer_create 240
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 241
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 244
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 245
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 246
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 260
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 261
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 281
# endif
# ifndef __NR_splice
#  define __NR_splice 283
# endif
# ifndef __NR_tee
#  define __NR_tee 284
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 285
# endif
# ifndef __NR_openat
#  define __NR_openat 286
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat (__NR_openat + 1)
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat (__NR_openat + 2)
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat (__NR_openat + 3)
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat (__NR_openat + 4)
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat (__NR_openat + 5)
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 (__NR_openat + 5)
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat (__NR_openat + 6)
# endif
# ifndef __NR_renameat
#  define __NR_renameat (__NR_openat + 7)
# endif
# ifndef __NR_linkat
#  define __NR_linkat (__NR_openat + 8)
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat (__NR_openat + 9)
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat (__NR_openat + 10)
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat (__NR_openat + 11)
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat (__NR_openat + 12)
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 304
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 307
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 309
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 314
# endif
#endif


#ifdef __powerpc__
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 0
# endif
# ifndef __NR_exit
#  define __NR_exit 1
# endif
# ifndef __NR_fork
#  define __NR_fork 2
# endif
# ifndef __NR_read
#  define __NR_read 3
# endif
# ifndef __NR_write
#  define __NR_write 4
# endif
# ifndef __NR_open
#  define __NR_open 5
# endif
# ifndef __NR_close
#  define __NR_close 6
# endif
# ifndef __NR_waitpid
#  define __NR_waitpid 7
# endif
# ifndef __NR_creat
#  define __NR_creat 8
# endif
# ifndef __NR_link
#  define __NR_link 9
# endif
# ifndef __NR_unlink
#  define __NR_unlink 10
# endif
# ifndef __NR_execve
#  define __NR_execve 11
# endif
# ifndef __NR_chdir
#  define __NR_chdir 12
# endif
# ifndef __NR_time
#  define __NR_time 13
# endif
# ifndef __NR_mknod
#  define __NR_mknod 14
# endif
# ifndef __NR_chmod
#  define __NR_chmod 15
# endif
# ifndef __NR_lchown
#  define __NR_lchown 16
# endif
# ifndef __NR_break
#  define __NR_break 17
# endif
# ifndef __NR_oldstat
#  define __NR_oldstat 18
# endif
# ifndef __NR_lseek
#  define __NR_lseek 19
# endif
# ifndef __NR_getpid
#  define __NR_getpid 20
# endif
# ifndef __NR_mount
#  define __NR_mount 21
# endif
# ifndef __NR_umount
#  define __NR_umount 22
# endif
# ifndef __NR_setuid
#  define __NR_setuid 23
# endif
# ifndef __NR_getuid
#  define __NR_getuid 24
# endif
# ifndef __NR_stime
#  define __NR_stime 25
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 26
# endif
# ifndef __NR_alarm
#  define __NR_alarm 27
# endif
# ifndef __NR_oldfstat
#  define __NR_oldfstat 28
# endif
# ifndef __NR_pause
#  define __NR_pause 29
# endif
# ifndef __NR_utime
#  define __NR_utime 30
# endif
# ifndef __NR_stty
#  define __NR_stty 31
# endif
# ifndef __NR_gtty
#  define __NR_gtty 32
# endif
# ifndef __NR_access
#  define __NR_access 33
# endif
# ifndef __NR_nice
#  define __NR_nice 34
# endif
# ifndef __NR_ftime
#  define __NR_ftime 35
# endif
# ifndef __NR_sync
#  define __NR_sync 36
# endif
# ifndef __NR_kill
#  define __NR_kill 37
# endif
# ifndef __NR_rename
#  define __NR_rename 38
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 39
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 40
# endif
# ifndef __NR_dup
#  define __NR_dup 41
# endif
# ifndef __NR_pipe
#  define __NR_pipe 42
# endif
# ifndef __NR_times
#  define __NR_times 43
# endif
# ifndef __NR_prof
#  define __NR_prof 44
# endif
# ifndef __NR_brk
#  define __NR_brk 45
# endif
# ifndef __NR_setgid
#  define __NR_setgid 46
# endif
# ifndef __NR_getgid
#  define __NR_getgid 47
# endif
# ifndef __NR_signal
#  define __NR_signal 48
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 49
# endif
# ifndef __NR_getegid
#  define __NR_getegid 50
# endif
# ifndef __NR_acct
#  define __NR_acct 51
# endif
# ifndef __NR_umount2
#  define __NR_umount2 52
# endif
# ifndef __NR_lock
#  define __NR_lock 53
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 54
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 55
# endif
# ifndef __NR_mpx
#  define __NR_mpx 56
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 57
# endif
# ifndef __NR_ulimit
#  define __NR_ulimit 58
# endif
# ifndef __NR_oldolduname
#  define __NR_oldolduname 59
# endif
# ifndef __NR_umask
#  define __NR_umask 60
# endif
# ifndef __NR_chroot
#  define __NR_chroot 61
# endif
# ifndef __NR_ustat
#  define __NR_ustat 62
# endif
# ifndef __NR_dup2
#  define __NR_dup2 63
# endif
# ifndef __NR_getppid
#  define __NR_getppid 64
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 65
# endif
# ifndef __NR_setsid
#  define __NR_setsid 66
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 67
# endif
# ifndef __NR_sgetmask
#  define __NR_sgetmask 68
# endif
# ifndef __NR_ssetmask
#  define __NR_ssetmask 69
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 70
# endif
# ifndef __NR_setregid
#  define __NR_setregid 71
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 72
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 73
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 74
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 75
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 76
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 77
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 78
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 79
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 80
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 81
# endif
# ifndef __NR_select
#  define __NR_select 82
# endif
# ifndef __NR_symlink
#  define __NR_symlink 83
# endif
# ifndef __NR_oldlstat
#  define __NR_oldlstat 84
# endif
# ifndef __NR_readlink
#  define __NR_readlink 85
# endif
# ifndef __NR_uselib
#  define __NR_uselib 86
# endif
# ifndef __NR_swapon
#  define __NR_swapon 87
# endif
# ifndef __NR_reboot
#  define __NR_reboot 88
# endif
# ifndef __NR_readdir
#  define __NR_readdir 89
# endif
# ifndef __NR_mmap
#  define __NR_mmap 90
# endif
# ifndef __NR_munmap
#  define __NR_munmap 91
# endif
# ifndef __NR_truncate
#  define __NR_truncate 92
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 93
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 94
# endif
# ifndef __NR_fchown
#  define __NR_fchown 95
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 96
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 97
# endif
# ifndef __NR_profil
#  define __NR_profil 98
# endif
# ifndef __NR_statfs
#  define __NR_statfs 99
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 100
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 101
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 102
# endif
# ifndef __NR_syslog
#  define __NR_syslog 103
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 104
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 105
# endif
# ifndef __NR_stat
#  define __NR_stat 106
# endif
# ifndef __NR_lstat
#  define __NR_lstat 107
# endif
# ifndef __NR_fstat
#  define __NR_fstat 108
# endif
# ifndef __NR_olduname
#  define __NR_olduname 109
# endif
# ifndef __NR_iopl
#  define __NR_iopl 110
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 111
# endif
# ifndef __NR_idle
#  define __NR_idle 112
# endif
# ifndef __NR_vm86
#  define __NR_vm86 113
# endif
# ifndef __NR_wait4
#  define __NR_wait4 114
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 115
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 116
# endif
# ifndef __NR_ipc
#  define __NR_ipc 117
# endif
# ifndef __NR_fsync
#  define __NR_fsync 118
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 119
# endif
# ifndef __NR_clone
#  define __NR_clone 120
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 121
# endif
# ifndef __NR_uname
#  define __NR_uname 122
# endif
# ifndef __NR_modify_ldt
#  define __NR_modify_ldt 123
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 124
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 125
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 126
# endif
# ifndef __NR_create_module
#  define __NR_create_module 127
# endif
# ifndef __NR_init_module
#  define __NR_init_module 128
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 129
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 130
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 131
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 132
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 133
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 134
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 135
# endif
# ifndef __NR_personality
#  define __NR_personality 136
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 137
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 138
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 139
# endif
# ifndef __NR_getdents
#  define __NR_getdents 141
# endif
# ifndef __NR_flock
#  define __NR_flock 143
# endif
# ifndef __NR_msync
#  define __NR_msync 144
# endif
# ifndef __NR_readv
#  define __NR_readv 145
# endif
# ifndef __NR_writev
#  define __NR_writev 146
# endif
# ifndef __NR_getsid
#  define __NR_getsid 147
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 148
# endif
# ifndef __NR_mlock
#  define __NR_mlock 150
# endif
# ifndef __NR_munlock
#  define __NR_munlock 151
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 152
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 153
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 154
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 155
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 156
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 157
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 158
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 159
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 160
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 161
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 162
# endif
# ifndef __NR_mremap
#  define __NR_mremap 163
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 164
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 165
# endif
# ifndef __NR_query_module
#  define __NR_query_module 166
# endif
# ifndef __NR_poll
#  define __NR_poll 167
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 168
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 169
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 170
# endif
# ifndef __NR_prctl
#  define __NR_prctl 171
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 172
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 173
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 174
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 175
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 176
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 177
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 178
# endif
# ifndef __NR_pread64
#  define __NR_pread64 179
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 180
# endif
# ifndef __NR_chown
#  define __NR_chown 181
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 182
# endif
# ifndef __NR_capget
#  define __NR_capget 183
# endif
# ifndef __NR_capset
#  define __NR_capset 184
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 185
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 186
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 187
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 188
# endif
# ifndef __NR_vfork
#  define __NR_vfork 189
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit 190
# endif
# ifndef __NR_readahead
#  define __NR_readahead 191
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 192
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 193
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 194
# endif
# ifndef __NR_stat64
#  define __NR_stat64 195
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 196
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 197
# endif
# ifndef __NR_pciconfig_read
#  define __NR_pciconfig_read 198
# endif
# ifndef __NR_pciconfig_write
#  define __NR_pciconfig_write 199
# endif
# ifndef __NR_pciconfig_iobase
#  define __NR_pciconfig_iobase 200
# endif
# ifndef __NR_multiplexer
#  define __NR_multiplexer 201
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 202
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 203
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 204
# endif
# ifndef __NR_madvise
#  define __NR_madvise 205
# endif
# ifndef __NR_mincore
#  define __NR_mincore 206
# endif
# ifndef __NR_gettid
#  define __NR_gettid 207
# endif
# ifndef __NR_tkill
#  define __NR_tkill 208
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 209
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 210
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 211
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 212
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 213
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 214
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 215
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 216
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 217
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 218
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 219
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 220
# endif
# ifndef __NR_futex
#  define __NR_futex 221
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 222
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 223
# endif
# ifndef __NR_tuxcall
#  define __NR_tuxcall 225
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 226
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 227
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 228
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 229
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 230
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 231
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 232
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 233
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 234
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 235
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 236
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 237
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 238
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 239
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 240
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 241
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime 242
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun 243
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 244
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 245
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 246
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres 247
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep 248
# endif
# ifndef __NR_swapcontext
#  define __NR_swapcontext 249
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 250
# endif
# ifndef __NR_utimes
#  define __NR_utimes 251
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 252
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 253
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 254
# endif
# ifndef __NR_rtas
#  define __NR_rtas 255
# endif
# ifndef __NR_sys_debug_setcontext
#  define __NR_sys_debug_setcontext 256
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 258
# endif
# ifndef __NR_mbind
#  define __NR_mbind 259
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 260
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 261
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 262
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 263
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 264
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 265
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 266
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 267
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 268
# endif
# ifndef __NR_add_key
#  define __NR_add_key 269
# endif
# ifndef __NR_request_key
#  define __NR_request_key 270
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 271
# endif
# ifndef __NR_waitid
#  define __NR_waitid 272
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 273
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 274
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 275
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 276
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 277
# endif
# ifndef __NR_spu_run
#  define __NR_spu_run 278
# endif
# ifndef __NR_spu_create
#  define __NR_spu_create 279
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 280
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 281
# endif
# ifndef __NR_unshare
#  define __NR_unshare 282
# endif
# ifndef __NR_splice
#  define __NR_splice 283
# endif
# ifndef __NR_tee
#  define __NR_tee 284
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 285
# endif
# ifndef __NR_openat
#  define __NR_openat 286
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 287
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 288
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 289
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 290
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 291
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 291
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 292
# endif
# ifndef __NR_renameat
#  define __NR_renameat 293
# endif
# ifndef __NR_linkat
#  define __NR_linkat 294
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 295
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 296
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 297
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 298
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 299
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 300
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 301
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 302
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 303
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 304
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 305
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 306
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 307
# endif
# ifndef __NR_sync_file_range2
#  define __NR_sync_file_range2 308
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 309
# endif
# ifndef __NR_subpage_prot
#  define __NR_subpage_prot 310
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 311
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 312
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 313
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 314
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 315
# endif
# ifndef __NR_dup3
#  define __NR_dup3 316
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 317
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 318
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 319
# endif
# ifndef __NR_preadv
#  define __NR_preadv 320
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 321
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 322
# endif
# ifndef __NR_syscalls
#  define __NR_syscalls 323
# endif
#endif


#ifdef __s390x__
# ifndef __NR_openat
#  define __NR_openat 288
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat (__NR_openat + 1)
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat (__NR_openat + 2)
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat (__NR_openat + 3)
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat (__NR_openat + 4)
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat (__NR_openat + 5)
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 (__NR_openat + 5)
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat (__NR_openat + 6)
# endif
# ifndef __NR_renameat
#  define __NR_renameat (__NR_openat + 7)
# endif
# ifndef __NR_linkat
#  define __NR_linkat (__NR_openat + 8)
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat (__NR_openat + 9)
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat (__NR_openat + 10)
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat (__NR_openat + 11)
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat (__NR_openat + 12)
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 318
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __s390__
# ifndef __NR_exit
#  define __NR_exit 1
# endif
# ifndef __NR_fork
#  define __NR_fork 2
# endif
# ifndef __NR_read
#  define __NR_read 3
# endif
# ifndef __NR_write
#  define __NR_write 4
# endif
# ifndef __NR_open
#  define __NR_open 5
# endif
# ifndef __NR_close
#  define __NR_close 6
# endif
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 7
# endif
# ifndef __NR_creat
#  define __NR_creat 8
# endif
# ifndef __NR_link
#  define __NR_link 9
# endif
# ifndef __NR_unlink
#  define __NR_unlink 10
# endif
# ifndef __NR_execve
#  define __NR_execve 11
# endif
# ifndef __NR_chdir
#  define __NR_chdir 12
# endif
# ifndef __NR_mknod
#  define __NR_mknod 14
# endif
# ifndef __NR_chmod
#  define __NR_chmod 15
# endif
# ifndef __NR_lseek
#  define __NR_lseek 19
# endif
# ifndef __NR_getpid
#  define __NR_getpid 20
# endif
# ifndef __NR_mount
#  define __NR_mount 21
# endif
# ifndef __NR_umount
#  define __NR_umount 22
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 26
# endif
# ifndef __NR_alarm
#  define __NR_alarm 27
# endif
# ifndef __NR_pause
#  define __NR_pause 29
# endif
# ifndef __NR_utime
#  define __NR_utime 30
# endif
# ifndef __NR_access
#  define __NR_access 33
# endif
# ifndef __NR_nice
#  define __NR_nice 34
# endif
# ifndef __NR_sync
#  define __NR_sync 36
# endif
# ifndef __NR_kill
#  define __NR_kill 37
# endif
# ifndef __NR_rename
#  define __NR_rename 38
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 39
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 40
# endif
# ifndef __NR_dup
#  define __NR_dup 41
# endif
# ifndef __NR_pipe
#  define __NR_pipe 42
# endif
# ifndef __NR_times
#  define __NR_times 43
# endif
# ifndef __NR_brk
#  define __NR_brk 45
# endif
# ifndef __NR_signal
#  define __NR_signal 48
# endif
# ifndef __NR_acct
#  define __NR_acct 51
# endif
# ifndef __NR_umount2
#  define __NR_umount2 52
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 54
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 55
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 57
# endif
# ifndef __NR_umask
#  define __NR_umask 60
# endif
# ifndef __NR_chroot
#  define __NR_chroot 61
# endif
# ifndef __NR_ustat
#  define __NR_ustat 62
# endif
# ifndef __NR_dup2
#  define __NR_dup2 63
# endif
# ifndef __NR_getppid
#  define __NR_getppid 64
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 65
# endif
# ifndef __NR_setsid
#  define __NR_setsid 66
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 67
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 72
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 73
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 74
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 75
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 77
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 78
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 79
# endif
# ifndef __NR_symlink
#  define __NR_symlink 83
# endif
# ifndef __NR_readlink
#  define __NR_readlink 85
# endif
# ifndef __NR_uselib
#  define __NR_uselib 86
# endif
# ifndef __NR_swapon
#  define __NR_swapon 87
# endif
# ifndef __NR_reboot
#  define __NR_reboot 88
# endif
# ifndef __NR_readdir
#  define __NR_readdir 89
# endif
# ifndef __NR_mmap
#  define __NR_mmap 90
# endif
# ifndef __NR_munmap
#  define __NR_munmap 91
# endif
# ifndef __NR_truncate
#  define __NR_truncate 92
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 93
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 94
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 96
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 97
# endif
# ifndef __NR_statfs
#  define __NR_statfs 99
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 100
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 102
# endif
# ifndef __NR_syslog
#  define __NR_syslog 103
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 104
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 105
# endif
# ifndef __NR_stat
#  define __NR_stat 106
# endif
# ifndef __NR_lstat
#  define __NR_lstat 107
# endif
# ifndef __NR_fstat
#  define __NR_fstat 108
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 110
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 111
# endif
# ifndef __NR_idle
#  define __NR_idle 112
# endif
# ifndef __NR_wait4
#  define __NR_wait4 114
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 115
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 116
# endif
# ifndef __NR_ipc
#  define __NR_ipc 117
# endif
# ifndef __NR_fsync
#  define __NR_fsync 118
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 119
# endif
# ifndef __NR_clone
#  define __NR_clone 120
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 121
# endif
# ifndef __NR_uname
#  define __NR_uname 122
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 124
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 125
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 126
# endif
# ifndef __NR_create_module
#  define __NR_create_module 127
# endif
# ifndef __NR_init_module
#  define __NR_init_module 128
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 129
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 130
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 131
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 132
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 133
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 134
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 135
# endif
# ifndef __NR_personality
#  define __NR_personality 136
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 137
# endif
# ifndef __NR_getdents
#  define __NR_getdents 141
# endif
# ifndef __NR_flock
#  define __NR_flock 143
# endif
# ifndef __NR_msync
#  define __NR_msync 144
# endif
# ifndef __NR_readv
#  define __NR_readv 145
# endif
# ifndef __NR_writev
#  define __NR_writev 146
# endif
# ifndef __NR_getsid
#  define __NR_getsid 147
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 148
# endif
# ifndef __NR_mlock
#  define __NR_mlock 150
# endif
# ifndef __NR_munlock
#  define __NR_munlock 151
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 152
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 153
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 154
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 155
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 156
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 157
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 158
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 159
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 160
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 161
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 162
# endif
# ifndef __NR_mremap
#  define __NR_mremap 163
# endif
# ifndef __NR_query_module
#  define __NR_query_module 167
# endif
# ifndef __NR_poll
#  define __NR_poll 168
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 169
# endif
# ifndef __NR_prctl
#  define __NR_prctl 172
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 173
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 174
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 175
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 176
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 177
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 178
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 179
# endif
# ifndef __NR_pread64
#  define __NR_pread64 180
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 181
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 183
# endif
# ifndef __NR_capget
#  define __NR_capget 184
# endif
# ifndef __NR_capset
#  define __NR_capset 185
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 186
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 187
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 188
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 189
# endif
# ifndef __NR_vfork
#  define __NR_vfork 190
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 217
# endif
# ifndef __NR_mincore
#  define __NR_mincore 218
# endif
# ifndef __NR_madvise
#  define __NR_madvise 219
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 220
# endif
# ifndef __NR_readahead
#  define __NR_readahead 222
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 224
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 225
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 226
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 227
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 228
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 229
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 230
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 231
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 232
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 233
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 234
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 235
# endif
# ifndef __NR_gettid
#  define __NR_gettid 236
# endif
# ifndef __NR_tkill
#  define __NR_tkill 237
# endif
# ifndef __NR_futex
#  define __NR_futex 238
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 239
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 240
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 241
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 243
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 244
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 245
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 246
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 247
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 248
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 249
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 250
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 251
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 252
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 253
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 254
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime (__NR_timer_create+1)
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime (__NR_timer_create+2)
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun (__NR_timer_create+3)
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete (__NR_timer_create+4)
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime (__NR_timer_create+5)
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime (__NR_timer_create+6)
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres (__NR_timer_create+7)
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep (__NR_timer_create+8)
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 265
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 266
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 267
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 271
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 272
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 273
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 274
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 275
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 276
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 277
# endif
# ifndef __NR_add_key
#  define __NR_add_key 278
# endif
# ifndef __NR_request_key
#  define __NR_request_key 279
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 280
# endif
# ifndef __NR_waitid
#  define __NR_waitid 281
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 282
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 283
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 284
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 285
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 286
# endif
# ifndef __NR_openat
#  define __NR_openat 288
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 289
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 290
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 291
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 292
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 294
# endif
# ifndef __NR_renameat
#  define __NR_renameat 295
# endif
# ifndef __NR_linkat
#  define __NR_linkat 296
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 297
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 298
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 299
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 300
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 301
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 302
# endif
# ifndef __NR_unshare
#  define __NR_unshare 303
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 304
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 305
# endif
# ifndef __NR_splice
#  define __NR_splice 306
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 307
# endif
# ifndef __NR_tee
#  define __NR_tee 308
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 309
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 311
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 312
# endif
# ifndef __NR_utimes
#  define __NR_utimes 313
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 315
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 316
# endif
# ifndef __NR_timerfd
#  define __NR_timerfd 317
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 318
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 319
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 320
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 321
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 322
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 323
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 324
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 325
# endif
# ifndef __NR_dup3
#  define __NR_dup3 326
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 327
# endif
# ifndef __NR_preadv
#  define __NR_preadv 328
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 329
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 330
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 331
# endif
# ifndef __NR_time
#  define __NR_time 13
# endif
# ifndef __NR_lchown
#  define __NR_lchown 16
# endif
# ifndef __NR_setuid
#  define __NR_setuid 23
# endif
# ifndef __NR_getuid
#  define __NR_getuid 24
# endif
# ifndef __NR_stime
#  define __NR_stime 25
# endif
# ifndef __NR_setgid
#  define __NR_setgid 46
# endif
# ifndef __NR_getgid
#  define __NR_getgid 47
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 49
# endif
# ifndef __NR_getegid
#  define __NR_getegid 50
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 70
# endif
# ifndef __NR_setregid
#  define __NR_setregid 71
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 76
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 80
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 81
# endif
# ifndef __NR_fchown
#  define __NR_fchown 95
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 101
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 138
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 139
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 164
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 165
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 170
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 171
# endif
# ifndef __NR_chown
#  define __NR_chown 182
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit 191
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 192
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 193
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 194
# endif
# ifndef __NR_stat64
#  define __NR_stat64 195
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 196
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 197
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 198
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 199
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 200
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 201
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 202
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 203
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 204
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 205
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 206
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 207
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 208
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 209
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 210
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 211
# endif
# ifndef __NR_chown32
#  define __NR_chown32 212
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 213
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 214
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 215
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 216
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 221
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 223
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 264
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 293
# endif
# ifndef __NR_select
#  define __NR_select 142
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 191
# endif
# ifndef __NR_lchown
#  define __NR_lchown 198
# endif
# ifndef __NR_getuid
#  define __NR_getuid 199
# endif
# ifndef __NR_getgid
#  define __NR_getgid 200
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 201
# endif
# ifndef __NR_getegid
#  define __NR_getegid 202
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 203
# endif
# ifndef __NR_setregid
#  define __NR_setregid 204
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 205
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 206
# endif
# ifndef __NR_fchown
#  define __NR_fchown 207
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 208
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 209
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 210
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 211
# endif
# ifndef __NR_chown
#  define __NR_chown 212
# endif
# ifndef __NR_setuid
#  define __NR_setuid 213
# endif
# ifndef __NR_setgid
#  define __NR_setgid 214
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 215
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 216
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 293
# endif
#endif


#ifdef __sh__
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 0
# endif
# ifndef __NR_exit
#  define __NR_exit 1
# endif
# ifndef __NR_fork
#  define __NR_fork 2
# endif
# ifndef __NR_read
#  define __NR_read 3
# endif
# ifndef __NR_write
#  define __NR_write 4
# endif
# ifndef __NR_open
#  define __NR_open 5
# endif
# ifndef __NR_close
#  define __NR_close 6
# endif
# ifndef __NR_waitpid
#  define __NR_waitpid 7
# endif
# ifndef __NR_creat
#  define __NR_creat 8
# endif
# ifndef __NR_link
#  define __NR_link 9
# endif
# ifndef __NR_unlink
#  define __NR_unlink 10
# endif
# ifndef __NR_execve
#  define __NR_execve 11
# endif
# ifndef __NR_chdir
#  define __NR_chdir 12
# endif
# ifndef __NR_time
#  define __NR_time 13
# endif
# ifndef __NR_mknod
#  define __NR_mknod 14
# endif
# ifndef __NR_chmod
#  define __NR_chmod 15
# endif
# ifndef __NR_lchown
#  define __NR_lchown 16
# endif
# ifndef __NR_break
#  define __NR_break 17
# endif
# ifndef __NR_oldstat
#  define __NR_oldstat 18
# endif
# ifndef __NR_lseek
#  define __NR_lseek 19
# endif
# ifndef __NR_getpid
#  define __NR_getpid 20
# endif
# ifndef __NR_mount
#  define __NR_mount 21
# endif
# ifndef __NR_umount
#  define __NR_umount 22
# endif
# ifndef __NR_setuid
#  define __NR_setuid 23
# endif
# ifndef __NR_getuid
#  define __NR_getuid 24
# endif
# ifndef __NR_stime
#  define __NR_stime 25
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 26
# endif
# ifndef __NR_alarm
#  define __NR_alarm 27
# endif
# ifndef __NR_oldfstat
#  define __NR_oldfstat 28
# endif
# ifndef __NR_pause
#  define __NR_pause 29
# endif
# ifndef __NR_utime
#  define __NR_utime 30
# endif
# ifndef __NR_stty
#  define __NR_stty 31
# endif
# ifndef __NR_gtty
#  define __NR_gtty 32
# endif
# ifndef __NR_access
#  define __NR_access 33
# endif
# ifndef __NR_nice
#  define __NR_nice 34
# endif
# ifndef __NR_ftime
#  define __NR_ftime 35
# endif
# ifndef __NR_sync
#  define __NR_sync 36
# endif
# ifndef __NR_kill
#  define __NR_kill 37
# endif
# ifndef __NR_rename
#  define __NR_rename 38
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 39
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 40
# endif
# ifndef __NR_dup
#  define __NR_dup 41
# endif
# ifndef __NR_pipe
#  define __NR_pipe 42
# endif
# ifndef __NR_times
#  define __NR_times 43
# endif
# ifndef __NR_prof
#  define __NR_prof 44
# endif
# ifndef __NR_brk
#  define __NR_brk 45
# endif
# ifndef __NR_setgid
#  define __NR_setgid 46
# endif
# ifndef __NR_getgid
#  define __NR_getgid 47
# endif
# ifndef __NR_signal
#  define __NR_signal 48
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 49
# endif
# ifndef __NR_getegid
#  define __NR_getegid 50
# endif
# ifndef __NR_acct
#  define __NR_acct 51
# endif
# ifndef __NR_umount2
#  define __NR_umount2 52
# endif
# ifndef __NR_lock
#  define __NR_lock 53
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 54
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 55
# endif
# ifndef __NR_mpx
#  define __NR_mpx 56
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 57
# endif
# ifndef __NR_ulimit
#  define __NR_ulimit 58
# endif
# ifndef __NR_oldolduname
#  define __NR_oldolduname 59
# endif
# ifndef __NR_umask
#  define __NR_umask 60
# endif
# ifndef __NR_chroot
#  define __NR_chroot 61
# endif
# ifndef __NR_ustat
#  define __NR_ustat 62
# endif
# ifndef __NR_dup2
#  define __NR_dup2 63
# endif
# ifndef __NR_getppid
#  define __NR_getppid 64
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 65
# endif
# ifndef __NR_setsid
#  define __NR_setsid 66
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 67
# endif
# ifndef __NR_sgetmask
#  define __NR_sgetmask 68
# endif
# ifndef __NR_ssetmask
#  define __NR_ssetmask 69
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 70
# endif
# ifndef __NR_setregid
#  define __NR_setregid 71
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 72
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 73
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 74
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 75
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 76
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 77
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 78
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 79
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 80
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 81
# endif
# ifndef __NR_select
#  define __NR_select 82
# endif
# ifndef __NR_symlink
#  define __NR_symlink 83
# endif
# ifndef __NR_oldlstat
#  define __NR_oldlstat 84
# endif
# ifndef __NR_readlink
#  define __NR_readlink 85
# endif
# ifndef __NR_uselib
#  define __NR_uselib 86
# endif
# ifndef __NR_swapon
#  define __NR_swapon 87
# endif
# ifndef __NR_reboot
#  define __NR_reboot 88
# endif
# ifndef __NR_readdir
#  define __NR_readdir 89
# endif
# ifndef __NR_mmap
#  define __NR_mmap 90
# endif
# ifndef __NR_munmap
#  define __NR_munmap 91
# endif
# ifndef __NR_truncate
#  define __NR_truncate 92
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 93
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 94
# endif
# ifndef __NR_fchown
#  define __NR_fchown 95
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 96
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 97
# endif
# ifndef __NR_profil
#  define __NR_profil 98
# endif
# ifndef __NR_statfs
#  define __NR_statfs 99
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 100
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 101
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 102
# endif
# ifndef __NR_syslog
#  define __NR_syslog 103
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 104
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 105
# endif
# ifndef __NR_stat
#  define __NR_stat 106
# endif
# ifndef __NR_lstat
#  define __NR_lstat 107
# endif
# ifndef __NR_fstat
#  define __NR_fstat 108
# endif
# ifndef __NR_olduname
#  define __NR_olduname 109
# endif
# ifndef __NR_iopl
#  define __NR_iopl 110
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 111
# endif
# ifndef __NR_idle
#  define __NR_idle 112
# endif
# ifndef __NR_vm86old
#  define __NR_vm86old 113
# endif
# ifndef __NR_wait4
#  define __NR_wait4 114
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 115
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 116
# endif
# ifndef __NR_ipc
#  define __NR_ipc 117
# endif
# ifndef __NR_fsync
#  define __NR_fsync 118
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 119
# endif
# ifndef __NR_clone
#  define __NR_clone 120
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 121
# endif
# ifndef __NR_uname
#  define __NR_uname 122
# endif
# ifndef __NR_cacheflush
#  define __NR_cacheflush 123
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 124
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 125
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 126
# endif
# ifndef __NR_create_module
#  define __NR_create_module 127
# endif
# ifndef __NR_init_module
#  define __NR_init_module 128
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 129
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 130
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 131
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 132
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 133
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 134
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 135
# endif
# ifndef __NR_personality
#  define __NR_personality 136
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 137
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 138
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 139
# endif
# ifndef __NR_getdents
#  define __NR_getdents 141
# endif
# ifndef __NR_flock
#  define __NR_flock 143
# endif
# ifndef __NR_msync
#  define __NR_msync 144
# endif
# ifndef __NR_readv
#  define __NR_readv 145
# endif
# ifndef __NR_writev
#  define __NR_writev 146
# endif
# ifndef __NR_getsid
#  define __NR_getsid 147
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 148
# endif
# ifndef __NR_mlock
#  define __NR_mlock 150
# endif
# ifndef __NR_munlock
#  define __NR_munlock 151
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 152
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 153
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 154
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 155
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 156
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 157
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 158
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 159
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 160
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 161
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 162
# endif
# ifndef __NR_mremap
#  define __NR_mremap 163
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 164
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 165
# endif
# ifndef __NR_vm86
#  define __NR_vm86 166
# endif
# ifndef __NR_query_module
#  define __NR_query_module 167
# endif
# ifndef __NR_poll
#  define __NR_poll 168
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 169
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 170
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 171
# endif
# ifndef __NR_prctl
#  define __NR_prctl 172
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 173
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 174
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 175
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 176
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 177
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 178
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 179
# endif
# ifndef __NR_pread64
#  define __NR_pread64 180
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 181
# endif
# ifndef __NR_chown
#  define __NR_chown 182
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 183
# endif
# ifndef __NR_capget
#  define __NR_capget 184
# endif
# ifndef __NR_capset
#  define __NR_capset 185
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 186
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 187
# endif
# ifndef __NR_streams1
#  define __NR_streams1 188
# endif
# ifndef __NR_streams2
#  define __NR_streams2 189
# endif
# ifndef __NR_vfork
#  define __NR_vfork 190
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit 191
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 192
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 193
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 194
# endif
# ifndef __NR_stat64
#  define __NR_stat64 195
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 196
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 197
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 198
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 199
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 200
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 201
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 202
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 203
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 204
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 205
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 206
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 207
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 208
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 209
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 210
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 211
# endif
# ifndef __NR_chown32
#  define __NR_chown32 212
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 213
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 214
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 215
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 216
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 217
# endif
# ifndef __NR_mincore
#  define __NR_mincore 218
# endif
# ifndef __NR_madvise
#  define __NR_madvise 219
# endif
# ifndef __NR_socket
#  define __NR_socket 220
# endif
# ifndef __NR_bind
#  define __NR_bind 221
# endif
# ifndef __NR_connect
#  define __NR_connect 222
# endif
# ifndef __NR_listen
#  define __NR_listen 223
# endif
# ifndef __NR_accept
#  define __NR_accept 224
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname 225
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername 226
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair 227
# endif
# ifndef __NR_send
#  define __NR_send 228
# endif
# ifndef __NR_sendto
#  define __NR_sendto 229
# endif
# ifndef __NR_recv
#  define __NR_recv 230
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom 231
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown 232
# endif
# ifndef __NR_setsockopt
#  define __NR_setsockopt 233
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt 234
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg 235
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg 236
# endif
# ifndef __NR_semop
#  define __NR_semop 237
# endif
# ifndef __NR_semget
#  define __NR_semget 238
# endif
# ifndef __NR_semctl
#  define __NR_semctl 239
# endif
# ifndef __NR_msgsnd
#  define __NR_msgsnd 240
# endif
# ifndef __NR_msgrcv
#  define __NR_msgrcv 241
# endif
# ifndef __NR_msgget
#  define __NR_msgget 242
# endif
# ifndef __NR_msgctl
#  define __NR_msgctl 243
# endif
# ifndef __NR_shmatcall
#  define __NR_shmatcall 244
# endif
# ifndef __NR_shmdt
#  define __NR_shmdt 245
# endif
# ifndef __NR_shmget
#  define __NR_shmget 246
# endif
# ifndef __NR_shmctl
#  define __NR_shmctl 247
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 248
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 249
# endif
# ifndef __NR_gettid
#  define __NR_gettid 252
# endif
# ifndef __NR_readahead
#  define __NR_readahead 253
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 254
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 255
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 256
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 257
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 258
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 269
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 260
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 261
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 262
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 263
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 264
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 265
# endif
# ifndef __NR_tkill
#  define __NR_tkill 266
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 267
# endif
# ifndef __NR_futex
#  define __NR_futex 268
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 269
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 270
# endif
# ifndef __NR_set_thread_area
#  define __NR_set_thread_area 271
# endif
# ifndef __NR_get_thread_area
#  define __NR_get_thread_area 272
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 273
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 274
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 275
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 276
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 277
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 278
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 280
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 281
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 282
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 283
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 284
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 285
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 286
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 287
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime (__NR_timer_create+1)
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime (__NR_timer_create+2)
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun (__NR_timer_create+3)
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete (__NR_timer_create+4)
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime (__NR_timer_create+5)
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime (__NR_timer_create+6)
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres (__NR_timer_create+7)
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep (__NR_timer_create+8)
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 296
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 297
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 298
# endif
# ifndef __NR_utimes
#  define __NR_utimes 299
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 300
# endif
# ifndef __NR_vserver
#  define __NR_vserver 301
# endif
# ifndef __NR_mbind
#  define __NR_mbind 302
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 303
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 304
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 305
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink (__NR_mq_open+1)
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend (__NR_mq_open+2)
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive (__NR_mq_open+3)
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify (__NR_mq_open+4)
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr (__NR_mq_open+5)
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 311
# endif
# ifndef __NR_waitid
#  define __NR_waitid 312
# endif
# ifndef __NR_add_key
#  define __NR_add_key 313
# endif
# ifndef __NR_request_key
#  define __NR_request_key 314
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 315
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 316
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 317
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 318
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 319
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 320
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 322
# endif
# ifndef __NR_openat
#  define __NR_openat 323
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 324
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 325
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 326
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 327
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 328
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 329
# endif
# ifndef __NR_renameat
#  define __NR_renameat 330
# endif
# ifndef __NR_linkat
#  define __NR_linkat 331
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 332
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 333
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 334
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 335
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 336
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 337
# endif
# ifndef __NR_unshare
#  define __NR_unshare 338
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 339
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 340
# endif
# ifndef __NR_splice
#  define __NR_splice 341
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 342
# endif
# ifndef __NR_tee
#  define __NR_tee 343
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 344
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 345
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 346
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 347
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 348
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 349
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 350
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 351
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 352
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 353
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 354
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 355
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 356
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 357
# endif
# ifndef __NR_dup3
#  define __NR_dup3 358
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 359
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 360
# endif
# ifndef __NR_preadv
#  define __NR_preadv 361
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 362
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 363
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 364
# endif
#endif


#ifdef __sparc64__
# ifndef __NR_vmsplice
#  define __NR_vmsplice 25
# endif
# ifndef __NR_splice
#  define __NR_splice 232
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 273
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 274
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 275
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 276
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 277
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 278
# endif
# ifndef __NR_tee
#  define __NR_tee 280
# endif
# ifndef __NR_add_key
#  define __NR_add_key 281
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 283
# endif
# ifndef __NR_openat
#  define __NR_openat 284
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat (__NR_openat + 1)
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat (__NR_openat + 2)
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat (__NR_openat + 3)
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat (__NR_openat + 4)
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat (__NR_openat + 5)
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 (__NR_openat + 5)
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat (__NR_openat + 6)
# endif
# ifndef __NR_renameat
#  define __NR_renameat (__NR_openat + 7)
# endif
# ifndef __NR_linkat
#  define __NR_linkat (__NR_openat + 8)
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat (__NR_openat + 9)
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat (__NR_openat + 10)
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat (__NR_openat + 11)
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat (__NR_openat + 12)
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 298
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 304
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 305
# endif
# ifndef __NR_eventd
#  define __NR_eventd 313
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __sparc__
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 0
# endif
# ifndef __NR_exit
#  define __NR_exit 1
# endif
# ifndef __NR_fork
#  define __NR_fork 2
# endif
# ifndef __NR_read
#  define __NR_read 3
# endif
# ifndef __NR_write
#  define __NR_write 4
# endif
# ifndef __NR_open
#  define __NR_open 5
# endif
# ifndef __NR_close
#  define __NR_close 6
# endif
# ifndef __NR_wait4
#  define __NR_wait4 7
# endif
# ifndef __NR_creat
#  define __NR_creat 8
# endif
# ifndef __NR_link
#  define __NR_link 9
# endif
# ifndef __NR_unlink
#  define __NR_unlink 10
# endif
# ifndef __NR_execv
#  define __NR_execv 11
# endif
# ifndef __NR_chdir
#  define __NR_chdir 12
# endif
# ifndef __NR_chown
#  define __NR_chown 13
# endif
# ifndef __NR_mknod
#  define __NR_mknod 14
# endif
# ifndef __NR_chmod
#  define __NR_chmod 15
# endif
# ifndef __NR_lchown
#  define __NR_lchown 16
# endif
# ifndef __NR_brk
#  define __NR_brk 17
# endif
# ifndef __NR_perfctr
#  define __NR_perfctr 18
# endif
# ifndef __NR_lseek
#  define __NR_lseek 19
# endif
# ifndef __NR_getpid
#  define __NR_getpid 20
# endif
# ifndef __NR_capget
#  define __NR_capget 21
# endif
# ifndef __NR_capset
#  define __NR_capset 22
# endif
# ifndef __NR_setuid
#  define __NR_setuid 23
# endif
# ifndef __NR_getuid
#  define __NR_getuid 24
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 25
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 26
# endif
# ifndef __NR_alarm
#  define __NR_alarm 27
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 28
# endif
# ifndef __NR_pause
#  define __NR_pause 29
# endif
# ifndef __NR_utime
#  define __NR_utime 30
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 31
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 32
# endif
# ifndef __NR_access
#  define __NR_access 33
# endif
# ifndef __NR_nice
#  define __NR_nice 34
# endif
# ifndef __NR_chown32
#  define __NR_chown32 35
# endif
# ifndef __NR_sync
#  define __NR_sync 36
# endif
# ifndef __NR_kill
#  define __NR_kill 37
# endif
# ifndef __NR_stat
#  define __NR_stat 38
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 39
# endif
# ifndef __NR_lstat
#  define __NR_lstat 40
# endif
# ifndef __NR_dup
#  define __NR_dup 41
# endif
# ifndef __NR_pipe
#  define __NR_pipe 42
# endif
# ifndef __NR_times
#  define __NR_times 43
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 44
# endif
# ifndef __NR_umount2
#  define __NR_umount2 45
# endif
# ifndef __NR_setgid
#  define __NR_setgid 46
# endif
# ifndef __NR_getgid
#  define __NR_getgid 47
# endif
# ifndef __NR_signal
#  define __NR_signal 48
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 49
# endif
# ifndef __NR_getegid
#  define __NR_getegid 50
# endif
# ifndef __NR_acct
#  define __NR_acct 51
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 53
# endif
# ifndef __NR_memory_ordering
#  define __NR_memory_ordering 52
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 54
# endif
# ifndef __NR_reboot
#  define __NR_reboot 55
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 56
# endif
# ifndef __NR_symlink
#  define __NR_symlink 57
# endif
# ifndef __NR_readlink
#  define __NR_readlink 58
# endif
# ifndef __NR_execve
#  define __NR_execve 59
# endif
# ifndef __NR_umask
#  define __NR_umask 60
# endif
# ifndef __NR_chroot
#  define __NR_chroot 61
# endif
# ifndef __NR_fstat
#  define __NR_fstat 62
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 63
# endif
# ifndef __NR_getpagesize
#  define __NR_getpagesize 64
# endif
# ifndef __NR_msync
#  define __NR_msync 65
# endif
# ifndef __NR_vfork
#  define __NR_vfork 66
# endif
# ifndef __NR_pread64
#  define __NR_pread64 67
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 68
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 69
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 70
# endif
# ifndef __NR_mmap
#  define __NR_mmap 71
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 72
# endif
# ifndef __NR_munmap
#  define __NR_munmap 73
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 74
# endif
# ifndef __NR_madvise
#  define __NR_madvise 75
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 76
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 77
# endif
# ifndef __NR_mincore
#  define __NR_mincore 78
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 79
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 80
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 81
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 82
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 83
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 84
# endif
# ifndef __NR_swapon
#  define __NR_swapon 85
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 86
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 87
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 88
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 89
# endif
# ifndef __NR_dup2
#  define __NR_dup2 90
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 91
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 92
# endif
# ifndef __NR_select
#  define __NR_select 93
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 94
# endif
# ifndef __NR_fsync
#  define __NR_fsync 95
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 96
# endif
# ifndef __NR_socket
#  define __NR_socket 97
# endif
# ifndef __NR_connect
#  define __NR_connect 98
# endif
# ifndef __NR_accept
#  define __NR_accept 99
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 100
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 101
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 102
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 103
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 104
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 105
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 106
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 107
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 108
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 109
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 110
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 111
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 112
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 108
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 109
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 110
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 111
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg 113
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg 114
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 115
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 116
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 117
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt 118
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 119
# endif
# ifndef __NR_readv
#  define __NR_readv 120
# endif
# ifndef __NR_writev
#  define __NR_writev 121
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 122
# endif
# ifndef __NR_fchown
#  define __NR_fchown 123
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 124
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom 125
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 126
# endif
# ifndef __NR_setregid
#  define __NR_setregid 127
# endif
# ifndef __NR_rename
#  define __NR_rename 128
# endif
# ifndef __NR_truncate
#  define __NR_truncate 129
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 130
# endif
# ifndef __NR_flock
#  define __NR_flock 131
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 132
# endif
# ifndef __NR_sendto
#  define __NR_sendto 133
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown 134
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair 135
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 136
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 137
# endif
# ifndef __NR_utimes
#  define __NR_utimes 138
# endif
# ifndef __NR_stat64
#  define __NR_stat64 139
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 140
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername 141
# endif
# ifndef __NR_futex
#  define __NR_futex 142
# endif
# ifndef __NR_gettid
#  define __NR_gettid 143
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 144
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 145
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 146
# endif
# ifndef __NR_prctl
#  define __NR_prctl 147
# endif
# ifndef __NR_pciconfig_read
#  define __NR_pciconfig_read 148
# endif
# ifndef __NR_pciconfig_write
#  define __NR_pciconfig_write 149
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname 150
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 151
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 152
# endif
# ifndef __NR_poll
#  define __NR_poll 153
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 154
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 155
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 156
# endif
# ifndef __NR_statfs
#  define __NR_statfs 157
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 158
# endif
# ifndef __NR_umount
#  define __NR_umount 159
# endif
# ifndef __NR_sched_set_affinity
#  define __NR_sched_set_affinity 160
# endif
# ifndef __NR_sched_get_affinity
#  define __NR_sched_get_affinity 161
# endif
# ifndef __NR_getdomainname
#  define __NR_getdomainname 162
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 163
# endif
# ifndef __NR_utrap_install
#  define __NR_utrap_install 164
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 165
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 166
# endif
# ifndef __NR_mount
#  define __NR_mount 167
# endif
# ifndef __NR_ustat
#  define __NR_ustat 168
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 169
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 170
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 171
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 172
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 173
# endif
# ifndef __NR_getdents
#  define __NR_getdents 174
# endif
# ifndef __NR_setsid
#  define __NR_setsid 175
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 176
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 177
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 178
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 179
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 180
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 181
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 182
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 183
# endif
# ifndef __NR_query_module
#  define __NR_query_module 184
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 185
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 186
# endif
# ifndef __NR_tkill
#  define __NR_tkill 187
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 188
# endif
# ifndef __NR_uname
#  define __NR_uname 189
# endif
# ifndef __NR_init_module
#  define __NR_init_module 190
# endif
# ifndef __NR_personality
#  define __NR_personality 191
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 192
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 193
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 194
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 195
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 196
# endif
# ifndef __NR_getppid
#  define __NR_getppid 197
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 198
# endif
# ifndef __NR_sgetmask
#  define __NR_sgetmask 199
# endif
# ifndef __NR_ssetmask
#  define __NR_ssetmask 200
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 201
# endif
# ifndef __NR_oldlstat
#  define __NR_oldlstat 202
# endif
# ifndef __NR_uselib
#  define __NR_uselib 203
# endif
# ifndef __NR_readdir
#  define __NR_readdir 204
# endif
# ifndef __NR_readahead
#  define __NR_readahead 205
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 206
# endif
# ifndef __NR_syslog
#  define __NR_syslog 207
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 208
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 209
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 210
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 211
# endif
# ifndef __NR_waitpid
#  define __NR_waitpid 212
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 213
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 214
# endif
# ifndef __NR_ipc
#  define __NR_ipc 215
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 216
# endif
# ifndef __NR_clone
#  define __NR_clone 217
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 218
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 219
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 220
# endif
# ifndef __NR_create_module
#  define __NR_create_module 221
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 222
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 223
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 224
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 225
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 226
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 227
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 228
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 229
# endif
# ifndef __NR_time
#  define __NR_time 231
# endif
# ifndef __NR_time
#  define __NR_time 231
# endif
# ifndef __NR_splice
#  define __NR_splice 232
# endif
# ifndef __NR_stime
#  define __NR_stime 233
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 234
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 235
# endif
# ifndef __NR_mlock
#  define __NR_mlock 237
# endif
# ifndef __NR_munlock
#  define __NR_munlock 238
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 239
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 240
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 241
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 242
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 243
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 244
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 245
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 246
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 247
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 248
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 249
# endif
# ifndef __NR_mremap
#  define __NR_mremap 250
# endif
# ifndef __NR_getsid
#  define __NR_getsid 252
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 253
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 254
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 255
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 256
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 257
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres 258
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep 259
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 260
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 261
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 262
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime 263
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun 264
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 265
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 266
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 268
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 269
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 270
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 271
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 272
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 273
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 274
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 275
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 276
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 277
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 278
# endif
# ifndef __NR_waitid
#  define __NR_waitid 279
# endif
# ifndef __NR_tee
#  define __NR_tee 280
# endif
# ifndef __NR_add_key
#  define __NR_add_key 281
# endif
# ifndef __NR_request_key
#  define __NR_request_key 282
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 283
# endif
# ifndef __NR_openat
#  define __NR_openat 284
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 285
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 286
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 287
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 288
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 289
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 290
# endif
# ifndef __NR_renameat
#  define __NR_renameat 291
# endif
# ifndef __NR_linkat
#  define __NR_linkat 292
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 293
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 294
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 295
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 296
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 297
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 298
# endif
# ifndef __NR_unshare
#  define __NR_unshare 299
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 300
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 301
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 302
# endif
# ifndef __NR_mbind
#  define __NR_mbind 303
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 304
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 305
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 306
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 307
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 308
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 309
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 310
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 311
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 312
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 313
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 315
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 316
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 317
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 318
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 319
# endif
# ifndef __NR_dup3
#  define __NR_dup3 320
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 321
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 322
# endif
# ifndef __NR_accept4
#  define __NR_accept4 323
# endif
# ifndef __NR_preadv
#  define __NR_preadv 324
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 325
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 326
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 327
# endif
#endif


#ifdef __x86_64__
# ifndef __NR_read
#  define __NR_read 0
# endif
# ifndef __NR_write
#  define __NR_write 1
# endif
# ifndef __NR_open
#  define __NR_open 2
# endif
# ifndef __NR_close
#  define __NR_close 3
# endif
# ifndef __NR_stat
#  define __NR_stat 4
# endif
# ifndef __NR_fstat
#  define __NR_fstat 5
# endif
# ifndef __NR_lstat
#  define __NR_lstat 6
# endif
# ifndef __NR_poll
#  define __NR_poll 7
# endif
# ifndef __NR_lseek
#  define __NR_lseek 8
# endif
# ifndef __NR_mmap
#  define __NR_mmap 9
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 10
# endif
# ifndef __NR_munmap
#  define __NR_munmap 11
# endif
# ifndef __NR_brk
#  define __NR_brk 12
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 13
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 14
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 15
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 16
# endif
# ifndef __NR_pread64
#  define __NR_pread64 17
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 18
# endif
# ifndef __NR_readv
#  define __NR_readv 19
# endif
# ifndef __NR_writev
#  define __NR_writev 20
# endif
# ifndef __NR_access
#  define __NR_access 21
# endif
# ifndef __NR_pipe
#  define __NR_pipe 22
# endif
# ifndef __NR_select
#  define __NR_select 23
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 24
# endif
# ifndef __NR_mremap
#  define __NR_mremap 25
# endif
# ifndef __NR_msync
#  define __NR_msync 26
# endif
# ifndef __NR_mincore
#  define __NR_mincore 27
# endif
# ifndef __NR_madvise
#  define __NR_madvise 28
# endif
# ifndef __NR_shmget
#  define __NR_shmget 29
# endif
# ifndef __NR_shmat
#  define __NR_shmat 30
# endif
# ifndef __NR_shmctl
#  define __NR_shmctl 31
# endif
# ifndef __NR_dup
#  define __NR_dup 32
# endif
# ifndef __NR_dup2
#  define __NR_dup2 33
# endif
# ifndef __NR_pause
#  define __NR_pause 34
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 35
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 36
# endif
# ifndef __NR_alarm
#  define __NR_alarm 37
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 38
# endif
# ifndef __NR_getpid
#  define __NR_getpid 39
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 40
# endif
# ifndef __NR_socket
#  define __NR_socket 41
# endif
# ifndef __NR_connect
#  define __NR_connect 42
# endif
# ifndef __NR_accept
#  define __NR_accept 43
# endif
# ifndef __NR_sendto
#  define __NR_sendto 44
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom 45
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg 46
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg 47
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown 48
# endif
# ifndef __NR_bind
#  define __NR_bind 49
# endif
# ifndef __NR_listen
#  define __NR_listen 50
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname 51
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername 52
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair 53
# endif
# ifndef __NR_setsockopt
#  define __NR_setsockopt 54
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt 55
# endif
# ifndef __NR_clone
#  define __NR_clone 56
# endif
# ifndef __NR_fork
#  define __NR_fork 57
# endif
# ifndef __NR_vfork
#  define __NR_vfork 58
# endif
# ifndef __NR_execve
#  define __NR_execve 59
# endif
# ifndef __NR_exit
#  define __NR_exit 60
# endif
# ifndef __NR_wait4
#  define __NR_wait4 61
# endif
# ifndef __NR_kill
#  define __NR_kill 62
# endif
# ifndef __NR_uname
#  define __NR_uname 63
# endif
# ifndef __NR_semget
#  define __NR_semget 64
# endif
# ifndef __NR_semop
#  define __NR_semop 65
# endif
# ifndef __NR_semctl
#  define __NR_semctl 66
# endif
# ifndef __NR_shmdt
#  define __NR_shmdt 67
# endif
# ifndef __NR_msgget
#  define __NR_msgget 68
# endif
# ifndef __NR_msgsnd
#  define __NR_msgsnd 69
# endif
# ifndef __NR_msgrcv
#  define __NR_msgrcv 70
# endif
# ifndef __NR_msgctl
#  define __NR_msgctl 71
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 72
# endif
# ifndef __NR_flock
#  define __NR_flock 73
# endif
# ifndef __NR_fsync
#  define __NR_fsync 74
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 75
# endif
# ifndef __NR_truncate
#  define __NR_truncate 76
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 77
# endif
# ifndef __NR_getdents
#  define __NR_getdents 78
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 79
# endif
# ifndef __NR_chdir
#  define __NR_chdir 80
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 81
# endif
# ifndef __NR_rename
#  define __NR_rename 82
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 83
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 84
# endif
# ifndef __NR_creat
#  define __NR_creat 85
# endif
# ifndef __NR_link
#  define __NR_link 86
# endif
# ifndef __NR_unlink
#  define __NR_unlink 87
# endif
# ifndef __NR_symlink
#  define __NR_symlink 88
# endif
# ifndef __NR_readlink
#  define __NR_readlink 89
# endif
# ifndef __NR_chmod
#  define __NR_chmod 90
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 91
# endif
# ifndef __NR_chown
#  define __NR_chown 92
# endif
# ifndef __NR_fchown
#  define __NR_fchown 93
# endif
# ifndef __NR_lchown
#  define __NR_lchown 94
# endif
# ifndef __NR_umask
#  define __NR_umask 95
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 96
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 97
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 98
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 99
# endif
# ifndef __NR_times
#  define __NR_times 100
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 101
# endif
# ifndef __NR_getuid
#  define __NR_getuid 102
# endif
# ifndef __NR_syslog
#  define __NR_syslog 103
# endif
# ifndef __NR_getgid
#  define __NR_getgid 104
# endif
# ifndef __NR_setuid
#  define __NR_setuid 105
# endif
# ifndef __NR_setgid
#  define __NR_setgid 106
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 107
# endif
# ifndef __NR_getegid
#  define __NR_getegid 108
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 109
# endif
# ifndef __NR_getppid
#  define __NR_getppid 110
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 111
# endif
# ifndef __NR_setsid
#  define __NR_setsid 112
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 113
# endif
# ifndef __NR_setregid
#  define __NR_setregid 114
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 115
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 116
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 117
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 118
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 119
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 120
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 121
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 122
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 123
# endif
# ifndef __NR_getsid
#  define __NR_getsid 124
# endif
# ifndef __NR_capget
#  define __NR_capget 125
# endif
# ifndef __NR_capset
#  define __NR_capset 126
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 127
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 128
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 129
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 130
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 131
# endif
# ifndef __NR_utime
#  define __NR_utime 132
# endif
# ifndef __NR_mknod
#  define __NR_mknod 133
# endif
# ifndef __NR_uselib
#  define __NR_uselib 134
# endif
# ifndef __NR_personality
#  define __NR_personality 135
# endif
# ifndef __NR_ustat
#  define __NR_ustat 136
# endif
# ifndef __NR_statfs
#  define __NR_statfs 137
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 138
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 139
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 140
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 141
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 142
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 143
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 144
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 145
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 146
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 147
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 148
# endif
# ifndef __NR_mlock
#  define __NR_mlock 149
# endif
# ifndef __NR_munlock
#  define __NR_munlock 150
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 151
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 152
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 153
# endif
# ifndef __NR_modify_ldt
#  define __NR_modify_ldt 154
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 155
# endif
# ifndef __NR_prctl
#  define __NR_prctl 157
# endif
# ifndef __NR_arch_prctl
#  define __NR_arch_prctl 158
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 159
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 160
# endif
# ifndef __NR_chroot
#  define __NR_chroot 161
# endif
# ifndef __NR_sync
#  define __NR_sync 162
# endif
# ifndef __NR_acct
#  define __NR_acct 163
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 164
# endif
# ifndef __NR_mount
#  define __NR_mount 165
# endif
# ifndef __NR_umount2
#  define __NR_umount2 166
# endif
# ifndef __NR_swapon
#  define __NR_swapon 167
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 168
# endif
# ifndef __NR_reboot
#  define __NR_reboot 169
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 170
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 171
# endif
# ifndef __NR_iopl
#  define __NR_iopl 172
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 173
# endif
# ifndef __NR_create_module
#  define __NR_create_module 174
# endif
# ifndef __NR_init_module
#  define __NR_init_module 175
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 176
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 177
# endif
# ifndef __NR_query_module
#  define __NR_query_module 178
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 179
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 180
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 181
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 182
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 183
# endif
# ifndef __NR_tuxcall
#  define __NR_tuxcall 184
# endif
# ifndef __NR_security
#  define __NR_security 185
# endif
# ifndef __NR_gettid
#  define __NR_gettid 186
# endif
# ifndef __NR_readahead
#  define __NR_readahead 187
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 188
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 189
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 190
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 191
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 192
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 193
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 194
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 195
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 196
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 197
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 198
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 199
# endif
# ifndef __NR_tkill
#  define __NR_tkill 200
# endif
# ifndef __NR_time
#  define __NR_time 201
# endif
# ifndef __NR_futex
#  define __NR_futex 202
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 203
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 204
# endif
# ifndef __NR_set_thread_area
#  define __NR_set_thread_area 205
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 206
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 207
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 208
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 209
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 210
# endif
# ifndef __NR_get_thread_area
#  define __NR_get_thread_area 211
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 212
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 213
# endif
# ifndef __NR_epoll_ctl_old
#  define __NR_epoll_ctl_old 214
# endif
# ifndef __NR_epoll_wait_old
#  define __NR_epoll_wait_old 215
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 216
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 217
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 218
# endif
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 219
# endif
# ifndef __NR_semtimedop
#  define __NR_semtimedop 220
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 221
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 222
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 223
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime 224
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun 225
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 226
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 227
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 228
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres 229
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep 230
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 231
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 232
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 233
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 234
# endif
# ifndef __NR_utimes
#  define __NR_utimes 235
# endif
# ifndef __NR_vserver
#  define __NR_vserver 236
# endif
# ifndef __NR_mbind
#  define __NR_mbind 237
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 238
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 239
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 240
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 241
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 242
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 243
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 244
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 245
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 246
# endif
# ifndef __NR_waitid
#  define __NR_waitid 247
# endif
# ifndef __NR_add_key
#  define __NR_add_key 248
# endif
# ifndef __NR_request_key
#  define __NR_request_key 249
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 250
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 251
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 252
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 253
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 254
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 255
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 256
# endif
# ifndef __NR_openat
#  define __NR_openat 257
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 258
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 259
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 260
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 261
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 262
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 263
# endif
# ifndef __NR_renameat
#  define __NR_renameat 264
# endif
# ifndef __NR_linkat
#  define __NR_linkat 265
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 266
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 267
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 268
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 269
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 270
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 271
# endif
# ifndef __NR_unshare
#  define __NR_unshare 272
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 273
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 274
# endif
# ifndef __NR_splice
#  define __NR_splice 275
# endif
# ifndef __NR_tee
#  define __NR_tee 276
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 277
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 278
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 279
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 280
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 281
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 282
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 283
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 284
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 285
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 286
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 287
# endif
# ifndef __NR_accept4
#  define __NR_accept4 288
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 289
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 290
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 291
# endif
# ifndef __NR_dup3
#  define __NR_dup3 292
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 293
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 294
# endif
# ifndef __NR_preadv
#  define __NR_preadv 295
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 296
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 297
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 298
# endif
#endif


/* Common stubs */
# ifndef __NR_accept
#  define __NR_accept 0
# endif
# ifndef __NR_accept4
#  define __NR_accept4 0
# endif
# ifndef __NR_access
#  define __NR_access 0
# endif
# ifndef __NR_acct
#  define __NR_acct 0
# endif
# ifndef __NR_add_key
#  define __NR_add_key 0
# endif
# ifndef __NR_adjtimex
#  define __NR_adjtimex 0
# endif
# ifndef __NR_afs_syscall
#  define __NR_afs_syscall 0
# endif
# ifndef __NR_alarm
#  define __NR_alarm 0
# endif
# ifndef __NR_arch_prctl
#  define __NR_arch_prctl 0
# endif
# ifndef __NR_arm_fadvise64_64
#  define __NR_arm_fadvise64_64 0
# endif
# ifndef __NR_arm_sync_file_range
#  define __NR_arm_sync_file_range 0
# endif
# ifndef __NR_bdflush
#  define __NR_bdflush 0
# endif
# ifndef __NR_bind
#  define __NR_bind 0
# endif
# ifndef __NR_break
#  define __NR_break 0
# endif
# ifndef __NR_brk
#  define __NR_brk 0
# endif
# ifndef __NR_cacheflush
#  define __NR_cacheflush 0
# endif
# ifndef __NR_capget
#  define __NR_capget 0
# endif
# ifndef __NR_capset
#  define __NR_capset 0
# endif
# ifndef __NR_chdir
#  define __NR_chdir 0
# endif
# ifndef __NR_chmod
#  define __NR_chmod 0
# endif
# ifndef __NR_chown
#  define __NR_chown 0
# endif
# ifndef __NR_chown32
#  define __NR_chown32 0
# endif
# ifndef __NR_chroot
#  define __NR_chroot 0
# endif
# ifndef __NR_clock_getres
#  define __NR_clock_getres 0
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 0
# endif
# ifndef __NR_clock_nanosleep
#  define __NR_clock_nanosleep 0
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 0
# endif
# ifndef __NR_clone
#  define __NR_clone 0
# endif
# ifndef __NR_clone2
#  define __NR_clone2 0
# endif
# ifndef __NR_close
#  define __NR_close 0
# endif
# ifndef __NR_connect
#  define __NR_connect 0
# endif
# ifndef __NR_creat
#  define __NR_creat 0
# endif
# ifndef __NR_create_module
#  define __NR_create_module 0
# endif
# ifndef __NR_delete_module
#  define __NR_delete_module 0
# endif
# ifndef __NR_dup
#  define __NR_dup 0
# endif
# ifndef __NR_dup2
#  define __NR_dup2 0
# endif
# ifndef __NR_dup3
#  define __NR_dup3 0
# endif
# ifndef __NR_epoll_create
#  define __NR_epoll_create 0
# endif
# ifndef __NR_epoll_create1
#  define __NR_epoll_create1 0
# endif
# ifndef __NR_epoll_ctl
#  define __NR_epoll_ctl 0
# endif
# ifndef __NR_epoll_ctl_old
#  define __NR_epoll_ctl_old 0
# endif
# ifndef __NR_epoll_pwait
#  define __NR_epoll_pwait 0
# endif
# ifndef __NR_epoll_wait
#  define __NR_epoll_wait 0
# endif
# ifndef __NR_epoll_wait_old
#  define __NR_epoll_wait_old 0
# endif
# ifndef __NR_eventd
#  define __NR_eventd 0
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd 0
# endif
# ifndef __NR_eventfd2
#  define __NR_eventfd2 0
# endif
# ifndef __NR_execv
#  define __NR_execv 0
# endif
# ifndef __NR_execve
#  define __NR_execve 0
# endif
# ifndef __NR_exit
#  define __NR_exit 0
# endif
# ifndef __NR_exit_group
#  define __NR_exit_group 0
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 0
# endif
# ifndef __NR_fadvise64
#  define __NR_fadvise64 0
# endif
# ifndef __NR_fadvise64_64
#  define __NR_fadvise64_64 0
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 0
# endif
# ifndef __NR_fchdir
#  define __NR_fchdir 0
# endif
# ifndef __NR_fchmod
#  define __NR_fchmod 0
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 0
# endif
# ifndef __NR_fchown
#  define __NR_fchown 0
# endif
# ifndef __NR_fchown32
#  define __NR_fchown32 0
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 0
# endif
# ifndef __NR_fcntl
#  define __NR_fcntl 0
# endif
# ifndef __NR_fcntl64
#  define __NR_fcntl64 0
# endif
# ifndef __NR_fdatasync
#  define __NR_fdatasync 0
# endif
# ifndef __NR_fgetxattr
#  define __NR_fgetxattr 0
# endif
# ifndef __NR_flistxattr
#  define __NR_flistxattr 0
# endif
# ifndef __NR_flock
#  define __NR_flock 0
# endif
# ifndef __NR_fork
#  define __NR_fork 0
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 0
# endif
# ifndef __NR_fsetxattr
#  define __NR_fsetxattr 0
# endif
# ifndef __NR_fstat
#  define __NR_fstat 0
# endif
# ifndef __NR_fstat64
#  define __NR_fstat64 0
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 0
# endif
# ifndef __NR_fstatfs
#  define __NR_fstatfs 0
# endif
# ifndef __NR_fstatfs64
#  define __NR_fstatfs64 0
# endif
# ifndef __NR_fsync
#  define __NR_fsync 0
# endif
# ifndef __NR_ftime
#  define __NR_ftime 0
# endif
# ifndef __NR_ftruncate
#  define __NR_ftruncate 0
# endif
# ifndef __NR_ftruncate64
#  define __NR_ftruncate64 0
# endif
# ifndef __NR_futex
#  define __NR_futex 0
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 0
# endif
# ifndef __NR_getcpu
#  define __NR_getcpu 0
# endif
# ifndef __NR_getcwd
#  define __NR_getcwd 0
# endif
# ifndef __NR_getdents
#  define __NR_getdents 0
# endif
# ifndef __NR_getdents64
#  define __NR_getdents64 0
# endif
# ifndef __NR_getdomainname
#  define __NR_getdomainname 0
# endif
# ifndef __NR_getegid
#  define __NR_getegid 0
# endif
# ifndef __NR_getegid32
#  define __NR_getegid32 0
# endif
# ifndef __NR_geteuid
#  define __NR_geteuid 0
# endif
# ifndef __NR_geteuid32
#  define __NR_geteuid32 0
# endif
# ifndef __NR_getgid
#  define __NR_getgid 0
# endif
# ifndef __NR_getgid32
#  define __NR_getgid32 0
# endif
# ifndef __NR_getgroups
#  define __NR_getgroups 0
# endif
# ifndef __NR_getgroups32
#  define __NR_getgroups32 0
# endif
# ifndef __NR_getitimer
#  define __NR_getitimer 0
# endif
# ifndef __NR_get_kernel_syms
#  define __NR_get_kernel_syms 0
# endif
# ifndef __NR_get_mempolicy
#  define __NR_get_mempolicy 0
# endif
# ifndef __NR_getpagesize
#  define __NR_getpagesize 0
# endif
# ifndef __NR_getpeername
#  define __NR_getpeername 0
# endif
# ifndef __NR_getpgid
#  define __NR_getpgid 0
# endif
# ifndef __NR_getpgrp
#  define __NR_getpgrp 0
# endif
# ifndef __NR_getpid
#  define __NR_getpid 0
# endif
# ifndef __NR_getpmsg
#  define __NR_getpmsg 0
# endif
# ifndef __NR_getppid
#  define __NR_getppid 0
# endif
# ifndef __NR_getpriority
#  define __NR_getpriority 0
# endif
# ifndef __NR_getresgid
#  define __NR_getresgid 0
# endif
# ifndef __NR_getresgid32
#  define __NR_getresgid32 0
# endif
# ifndef __NR_getresuid
#  define __NR_getresuid 0
# endif
# ifndef __NR_getresuid32
#  define __NR_getresuid32 0
# endif
# ifndef __NR_getrlimit
#  define __NR_getrlimit 0
# endif
# ifndef __NR_get_robust_list
#  define __NR_get_robust_list 0
# endif
# ifndef __NR_getrusage
#  define __NR_getrusage 0
# endif
# ifndef __NR_getsid
#  define __NR_getsid 0
# endif
# ifndef __NR_getsockname
#  define __NR_getsockname 0
# endif
# ifndef __NR_getsockopt
#  define __NR_getsockopt 0
# endif
# ifndef __NR_get_thread_area
#  define __NR_get_thread_area 0
# endif
# ifndef __NR_gettid
#  define __NR_gettid 0
# endif
# ifndef __NR_gettimeofday
#  define __NR_gettimeofday 0
# endif
# ifndef __NR_getuid
#  define __NR_getuid 0
# endif
# ifndef __NR_getuid32
#  define __NR_getuid32 0
# endif
# ifndef __NR_getunwind
#  define __NR_getunwind 0
# endif
# ifndef __NR_getxattr
#  define __NR_getxattr 0
# endif
# ifndef __NR_gtty
#  define __NR_gtty 0
# endif
# ifndef __NR_idle
#  define __NR_idle 0
# endif
# ifndef __NR_init_module
#  define __NR_init_module 0
# endif
# ifndef __NR_inotify_add_watch
#  define __NR_inotify_add_watch 0
# endif
# ifndef __NR_inotify_init
#  define __NR_inotify_init 0
# endif
# ifndef __NR_inotify_init1
#  define __NR_inotify_init1 0
# endif
# ifndef __NR_inotify_rm_watch
#  define __NR_inotify_rm_watch 0
# endif
# ifndef __NR_io_cancel
#  define __NR_io_cancel 0
# endif
# ifndef __NR_ioctl
#  define __NR_ioctl 0
# endif
# ifndef __NR_io_destroy
#  define __NR_io_destroy 0
# endif
# ifndef __NR_io_getevents
#  define __NR_io_getevents 0
# endif
# ifndef __NR_ioperm
#  define __NR_ioperm 0
# endif
# ifndef __NR_iopl
#  define __NR_iopl 0
# endif
# ifndef __NR_ioprio_get
#  define __NR_ioprio_get 0
# endif
# ifndef __NR_ioprio_set
#  define __NR_ioprio_set 0
# endif
# ifndef __NR_io_setup
#  define __NR_io_setup 0
# endif
# ifndef __NR_io_submit
#  define __NR_io_submit 0
# endif
# ifndef __NR_ipc
#  define __NR_ipc 0
# endif
# ifndef __NR_kexec_load
#  define __NR_kexec_load 0
# endif
# ifndef __NR_keyctl
#  define __NR_keyctl 0
# endif
# ifndef __NR_kill
#  define __NR_kill 0
# endif
# ifndef __NR_lchown
#  define __NR_lchown 0
# endif
# ifndef __NR_lchown32
#  define __NR_lchown32 0
# endif
# ifndef __NR_lgetxattr
#  define __NR_lgetxattr 0
# endif
# ifndef __NR_link
#  define __NR_link 0
# endif
# ifndef __NR_linkat
#  define __NR_linkat 0
# endif
# ifndef __NR_listen
#  define __NR_listen 0
# endif
# ifndef __NR_listxattr
#  define __NR_listxattr 0
# endif
# ifndef __NR_llistxattr
#  define __NR_llistxattr 0
# endif
# ifndef __NR_lock
#  define __NR_lock 0
# endif
# ifndef __NR_lookup_dcookie
#  define __NR_lookup_dcookie 0
# endif
# ifndef __NR_lremovexattr
#  define __NR_lremovexattr 0
# endif
# ifndef __NR_lseek
#  define __NR_lseek 0
# endif
# ifndef __NR_lsetxattr
#  define __NR_lsetxattr 0
# endif
# ifndef __NR_lstat
#  define __NR_lstat 0
# endif
# ifndef __NR_lstat64
#  define __NR_lstat64 0
# endif
# ifndef __NR_madvise
#  define __NR_madvise 0
# endif
# ifndef __NR_madvise1
#  define __NR_madvise1 0
# endif
# ifndef __NR_mbind
#  define __NR_mbind 0
# endif
# ifndef __NR_memory_ordering
#  define __NR_memory_ordering 0
# endif
# ifndef __NR_migrate_pages
#  define __NR_migrate_pages 0
# endif
# ifndef __NR_mincore
#  define __NR_mincore 0
# endif
# ifndef __NR_mkdir
#  define __NR_mkdir 0
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 0
# endif
# ifndef __NR_mknod
#  define __NR_mknod 0
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 0
# endif
# ifndef __NR_mlock
#  define __NR_mlock 0
# endif
# ifndef __NR_mlockall
#  define __NR_mlockall 0
# endif
# ifndef __NR_mmap
#  define __NR_mmap 0
# endif
# ifndef __NR_mmap2
#  define __NR_mmap2 0
# endif
# ifndef __NR_modify_ldt
#  define __NR_modify_ldt 0
# endif
# ifndef __NR_mount
#  define __NR_mount 0
# endif
# ifndef __NR_move_pages
#  define __NR_move_pages 0
# endif
# ifndef __NR_mprotect
#  define __NR_mprotect 0
# endif
# ifndef __NR_mpx
#  define __NR_mpx 0
# endif
# ifndef __NR_mq_getsetattr
#  define __NR_mq_getsetattr 0
# endif
# ifndef __NR_mq_notify
#  define __NR_mq_notify 0
# endif
# ifndef __NR_mq_open
#  define __NR_mq_open 0
# endif
# ifndef __NR_mq_timedreceive
#  define __NR_mq_timedreceive 0
# endif
# ifndef __NR_mq_timedsend
#  define __NR_mq_timedsend 0
# endif
# ifndef __NR_mq_unlink
#  define __NR_mq_unlink 0
# endif
# ifndef __NR_mremap
#  define __NR_mremap 0
# endif
# ifndef __NR_msgctl
#  define __NR_msgctl 0
# endif
# ifndef __NR_msgget
#  define __NR_msgget 0
# endif
# ifndef __NR_msgrcv
#  define __NR_msgrcv 0
# endif
# ifndef __NR_msgsnd
#  define __NR_msgsnd 0
# endif
# ifndef __NR_msync
#  define __NR_msync 0
# endif
# ifndef __NR_multiplexer
#  define __NR_multiplexer 0
# endif
# ifndef __NR_munlock
#  define __NR_munlock 0
# endif
# ifndef __NR_munlockall
#  define __NR_munlockall 0
# endif
# ifndef __NR_munmap
#  define __NR_munmap 0
# endif
# ifndef __NR_nanosleep
#  define __NR_nanosleep 0
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 0
# endif
# ifndef __NR_nfsservctl
#  define __NR_nfsservctl 0
# endif
# ifndef __NR_nice
#  define __NR_nice 0
# endif
# ifndef __NR_ni_syscall
#  define __NR_ni_syscall 0
# endif
# ifndef __NR_oldfstat
#  define __NR_oldfstat 0
# endif
# ifndef __NR_oldlstat
#  define __NR_oldlstat 0
# endif
# ifndef __NR_oldolduname
#  define __NR_oldolduname 0
# endif
# ifndef __NR_oldstat
#  define __NR_oldstat 0
# endif
# ifndef __NR_olduname
#  define __NR_olduname 0
# endif
# ifndef __NR_open
#  define __NR_open 0
# endif
# ifndef __NR_openat
#  define __NR_openat 0
# endif
# ifndef __NR_pause
#  define __NR_pause 0
# endif
# ifndef __NR_pciconfig_iobase
#  define __NR_pciconfig_iobase 0
# endif
# ifndef __NR_pciconfig_read
#  define __NR_pciconfig_read 0
# endif
# ifndef __NR_pciconfig_write
#  define __NR_pciconfig_write 0
# endif
# ifndef __NR_perfctr
#  define __NR_perfctr 0
# endif
# ifndef __NR_perf_event_open
#  define __NR_perf_event_open 0
# endif
# ifndef __NR_perfmonctl
#  define __NR_perfmonctl 0
# endif
# ifndef __NR_personality
#  define __NR_personality 0
# endif
# ifndef __NR_pipe
#  define __NR_pipe 0
# endif
# ifndef __NR_pipe2
#  define __NR_pipe2 0
# endif
# ifndef __NR_pivot_root
#  define __NR_pivot_root 0
# endif
# ifndef __NR_poll
#  define __NR_poll 0
# endif
# ifndef __NR_ppoll
#  define __NR_ppoll 0
# endif
# ifndef __NR_prctl
#  define __NR_prctl 0
# endif
# ifndef __NR_pread64
#  define __NR_pread64 0
# endif
# ifndef __NR_preadv
#  define __NR_preadv 0
# endif
# ifndef __NR_prof
#  define __NR_prof 0
# endif
# ifndef __NR_profil
#  define __NR_profil 0
# endif
# ifndef __NR_pselect6
#  define __NR_pselect6 0
# endif
# ifndef __NR_ptrace
#  define __NR_ptrace 0
# endif
# ifndef __NR_putpmsg
#  define __NR_putpmsg 0
# endif
# ifndef __NR_pwrite64
#  define __NR_pwrite64 0
# endif
# ifndef __NR_pwritev
#  define __NR_pwritev 0
# endif
# ifndef __NR_query_module
#  define __NR_query_module 0
# endif
# ifndef __NR_quotactl
#  define __NR_quotactl 0
# endif
# ifndef __NR_read
#  define __NR_read 0
# endif
# ifndef __NR_readahead
#  define __NR_readahead 0
# endif
# ifndef __NR_readdir
#  define __NR_readdir 0
# endif
# ifndef __NR_readlink
#  define __NR_readlink 0
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 0
# endif
# ifndef __NR_readv
#  define __NR_readv 0
# endif
# ifndef __NR_reboot
#  define __NR_reboot 0
# endif
# ifndef __NR_recv
#  define __NR_recv 0
# endif
# ifndef __NR_recvfrom
#  define __NR_recvfrom 0
# endif
# ifndef __NR_recvmsg
#  define __NR_recvmsg 0
# endif
# ifndef __NR_remap_file_pages
#  define __NR_remap_file_pages 0
# endif
# ifndef __NR_removexattr
#  define __NR_removexattr 0
# endif
# ifndef __NR_rename
#  define __NR_rename 0
# endif
# ifndef __NR_renameat
#  define __NR_renameat 0
# endif
# ifndef __NR_request_key
#  define __NR_request_key 0
# endif
# ifndef __NR_restart_syscall
#  define __NR_restart_syscall 0
# endif
# ifndef __NR_rmdir
#  define __NR_rmdir 0
# endif
# ifndef __NR_rtas
#  define __NR_rtas 0
# endif
# ifndef __NR_rt_sigaction
#  define __NR_rt_sigaction 0
# endif
# ifndef __NR_rt_sigpending
#  define __NR_rt_sigpending 0
# endif
# ifndef __NR_rt_sigprocmask
#  define __NR_rt_sigprocmask 0
# endif
# ifndef __NR_rt_sigqueueinfo
#  define __NR_rt_sigqueueinfo 0
# endif
# ifndef __NR_rt_sigreturn
#  define __NR_rt_sigreturn 0
# endif
# ifndef __NR_rt_sigsuspend
#  define __NR_rt_sigsuspend 0
# endif
# ifndef __NR_rt_sigtimedwait
#  define __NR_rt_sigtimedwait 0
# endif
# ifndef __NR_rt_tgsigqueueinfo
#  define __NR_rt_tgsigqueueinfo 0
# endif
# ifndef __NR_sched_getaffinity
#  define __NR_sched_getaffinity 0
# endif
# ifndef __NR_sched_get_affinity
#  define __NR_sched_get_affinity 0
# endif
# ifndef __NR_sched_getparam
#  define __NR_sched_getparam 0
# endif
# ifndef __NR_sched_get_priority_max
#  define __NR_sched_get_priority_max 0
# endif
# ifndef __NR_sched_get_priority_min
#  define __NR_sched_get_priority_min 0
# endif
# ifndef __NR_sched_getscheduler
#  define __NR_sched_getscheduler 0
# endif
# ifndef __NR_sched_rr_get_interval
#  define __NR_sched_rr_get_interval 0
# endif
# ifndef __NR_sched_setaffinity
#  define __NR_sched_setaffinity 0
# endif
# ifndef __NR_sched_set_affinity
#  define __NR_sched_set_affinity 0
# endif
# ifndef __NR_sched_setparam
#  define __NR_sched_setparam 0
# endif
# ifndef __NR_sched_setscheduler
#  define __NR_sched_setscheduler 0
# endif
# ifndef __NR_sched_yield
#  define __NR_sched_yield 0
# endif
# ifndef __NR_security
#  define __NR_security 0
# endif
# ifndef __NR_select
#  define __NR_select 0
# endif
# ifndef __NR_semctl
#  define __NR_semctl 0
# endif
# ifndef __NR_semget
#  define __NR_semget 0
# endif
# ifndef __NR_semop
#  define __NR_semop 0
# endif
# ifndef __NR_semtimedop
#  define __NR_semtimedop 0
# endif
# ifndef __NR_send
#  define __NR_send 0
# endif
# ifndef __NR_sendfile
#  define __NR_sendfile 0
# endif
# ifndef __NR_sendfile64
#  define __NR_sendfile64 0
# endif
# ifndef __NR_sendmsg
#  define __NR_sendmsg 0
# endif
# ifndef __NR_sendto
#  define __NR_sendto 0
# endif
# ifndef __NR_setdomainname
#  define __NR_setdomainname 0
# endif
# ifndef __NR_setfsgid
#  define __NR_setfsgid 0
# endif
# ifndef __NR_setfsgid32
#  define __NR_setfsgid32 0
# endif
# ifndef __NR_setfsuid
#  define __NR_setfsuid 0
# endif
# ifndef __NR_setfsuid32
#  define __NR_setfsuid32 0
# endif
# ifndef __NR_setgid
#  define __NR_setgid 0
# endif
# ifndef __NR_setgid32
#  define __NR_setgid32 0
# endif
# ifndef __NR_setgroups
#  define __NR_setgroups 0
# endif
# ifndef __NR_setgroups32
#  define __NR_setgroups32 0
# endif
# ifndef __NR_sethostname
#  define __NR_sethostname 0
# endif
# ifndef __NR_setitimer
#  define __NR_setitimer 0
# endif
# ifndef __NR_set_mempolicy
#  define __NR_set_mempolicy 0
# endif
# ifndef __NR_setpgid
#  define __NR_setpgid 0
# endif
# ifndef __NR_setpriority
#  define __NR_setpriority 0
# endif
# ifndef __NR_setregid
#  define __NR_setregid 0
# endif
# ifndef __NR_setregid32
#  define __NR_setregid32 0
# endif
# ifndef __NR_setresgid
#  define __NR_setresgid 0
# endif
# ifndef __NR_setresgid32
#  define __NR_setresgid32 0
# endif
# ifndef __NR_setresuid
#  define __NR_setresuid 0
# endif
# ifndef __NR_setresuid32
#  define __NR_setresuid32 0
# endif
# ifndef __NR_setreuid
#  define __NR_setreuid 0
# endif
# ifndef __NR_setreuid32
#  define __NR_setreuid32 0
# endif
# ifndef __NR_setrlimit
#  define __NR_setrlimit 0
# endif
# ifndef __NR_set_robust_list
#  define __NR_set_robust_list 0
# endif
# ifndef __NR_setsid
#  define __NR_setsid 0
# endif
# ifndef __NR_setsockopt
#  define __NR_setsockopt 0
# endif
# ifndef __NR_set_thread_area
#  define __NR_set_thread_area 0
# endif
# ifndef __NR_set_tid_address
#  define __NR_set_tid_address 0
# endif
# ifndef __NR_settimeofday
#  define __NR_settimeofday 0
# endif
# ifndef __NR_setuid
#  define __NR_setuid 0
# endif
# ifndef __NR_setuid32
#  define __NR_setuid32 0
# endif
# ifndef __NR_setxattr
#  define __NR_setxattr 0
# endif
# ifndef __NR_sgetmask
#  define __NR_sgetmask 0
# endif
# ifndef __NR_shmat
#  define __NR_shmat 0
# endif
# ifndef __NR_shmatcall
#  define __NR_shmatcall 0
# endif
# ifndef __NR_shmctl
#  define __NR_shmctl 0
# endif
# ifndef __NR_shmdt
#  define __NR_shmdt 0
# endif
# ifndef __NR_shmget
#  define __NR_shmget 0
# endif
# ifndef __NR_shutdown
#  define __NR_shutdown 0
# endif
# ifndef __NR_sigaction
#  define __NR_sigaction 0
# endif
# ifndef __NR_sigaltstack
#  define __NR_sigaltstack 0
# endif
# ifndef __NR_signal
#  define __NR_signal 0
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd 0
# endif
# ifndef __NR_signalfd4
#  define __NR_signalfd4 0
# endif
# ifndef __NR_sigpending
#  define __NR_sigpending 0
# endif
# ifndef __NR_sigprocmask
#  define __NR_sigprocmask 0
# endif
# ifndef __NR_sigreturn
#  define __NR_sigreturn 0
# endif
# ifndef __NR_sigsuspend
#  define __NR_sigsuspend 0
# endif
# ifndef __NR_socket
#  define __NR_socket 0
# endif
# ifndef __NR_socketcall
#  define __NR_socketcall 0
# endif
# ifndef __NR_socketpair
#  define __NR_socketpair 0
# endif
# ifndef __NR_splice
#  define __NR_splice 0
# endif
# ifndef __NR_spu_create
#  define __NR_spu_create 0
# endif
# ifndef __NR_spu_run
#  define __NR_spu_run 0
# endif
# ifndef __NR_ssetmask
#  define __NR_ssetmask 0
# endif
# ifndef __NR_stat
#  define __NR_stat 0
# endif
# ifndef __NR_stat64
#  define __NR_stat64 0
# endif
# ifndef __NR_statfs
#  define __NR_statfs 0
# endif
# ifndef __NR_statfs64
#  define __NR_statfs64 0
# endif
# ifndef __NR_stime
#  define __NR_stime 0
# endif
# ifndef __NR_streams1
#  define __NR_streams1 0
# endif
# ifndef __NR_streams2
#  define __NR_streams2 0
# endif
# ifndef __NR_stty
#  define __NR_stty 0
# endif
# ifndef __NR_subpage_prot
#  define __NR_subpage_prot 0
# endif
# ifndef __NR_swapcontext
#  define __NR_swapcontext 0
# endif
# ifndef __NR_swapoff
#  define __NR_swapoff 0
# endif
# ifndef __NR_swapon
#  define __NR_swapon 0
# endif
# ifndef __NR_symlink
#  define __NR_symlink 0
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 0
# endif
# ifndef __NR_sync
#  define __NR_sync 0
# endif
# ifndef __NR_sync_file_range
#  define __NR_sync_file_range 0
# endif
# ifndef __NR_sync_file_range2
#  define __NR_sync_file_range2 0
# endif
# ifndef __NR_syscall
#  define __NR_syscall 0
# endif
# ifndef __NR_syscalls
#  define __NR_syscalls 0
# endif
# ifndef __NR_sys_debug_setcontext
#  define __NR_sys_debug_setcontext 0
# endif
# ifndef __NR_sysfs
#  define __NR_sysfs 0
# endif
# ifndef __NR_sysinfo
#  define __NR_sysinfo 0
# endif
# ifndef __NR_syslog
#  define __NR_syslog 0
# endif
# ifndef __NR_tee
#  define __NR_tee 0
# endif
# ifndef __NR_tgkill
#  define __NR_tgkill 0
# endif
# ifndef __NR_time
#  define __NR_time 0
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 0
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 0
# endif
# ifndef __NR_timerfd
#  define __NR_timerfd 0
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 0
# endif
# ifndef __NR_timerfd_gettime
#  define __NR_timerfd_gettime 0
# endif
# ifndef __NR_timerfd_settime
#  define __NR_timerfd_settime 0
# endif
# ifndef __NR_timer_getoverrun
#  define __NR_timer_getoverrun 0
# endif
# ifndef __NR_timer_gettime
#  define __NR_timer_gettime 0
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 0
# endif
# ifndef __NR_times
#  define __NR_times 0
# endif
# ifndef __NR_tkill
#  define __NR_tkill 0
# endif
# ifndef __NR_truncate
#  define __NR_truncate 0
# endif
# ifndef __NR_truncate64
#  define __NR_truncate64 0
# endif
# ifndef __NR_tuxcall
#  define __NR_tuxcall 0
# endif
# ifndef __NR_ugetrlimit
#  define __NR_ugetrlimit 0
# endif
# ifndef __NR_ulimit
#  define __NR_ulimit 0
# endif
# ifndef __NR_umask
#  define __NR_umask 0
# endif
# ifndef __NR_umount
#  define __NR_umount 0
# endif
# ifndef __NR_umount2
#  define __NR_umount2 0
# endif
# ifndef __NR_uname
#  define __NR_uname 0
# endif
# ifndef __NR_unlink
#  define __NR_unlink 0
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 0
# endif
# ifndef __NR_unshare
#  define __NR_unshare 0
# endif
# ifndef __NR_uselib
#  define __NR_uselib 0
# endif
# ifndef __NR_ustat
#  define __NR_ustat 0
# endif
# ifndef __NR_utime
#  define __NR_utime 0
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 0
# endif
# ifndef __NR_utimes
#  define __NR_utimes 0
# endif
# ifndef __NR_utrap_install
#  define __NR_utrap_install 0
# endif
# ifndef __NR_vfork
#  define __NR_vfork 0
# endif
# ifndef __NR_vhangup
#  define __NR_vhangup 0
# endif
# ifndef __NR_vm86
#  define __NR_vm86 0
# endif
# ifndef __NR_vm86old
#  define __NR_vm86old 0
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 0
# endif
# ifndef __NR_vserver
#  define __NR_vserver 0
# endif
# ifndef __NR_wait4
#  define __NR_wait4 0
# endif
# ifndef __NR_waitid
#  define __NR_waitid 0
# endif
# ifndef __NR_waitpid
#  define __NR_waitpid 0
# endif
# ifndef __NR_write
#  define __NR_write 0
# endif
# ifndef __NR_writev
#  define __NR_writev 0
# endif
#endif
