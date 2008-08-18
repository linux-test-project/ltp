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

#include <sys/syscall.h>


#ifdef __arm__
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
# ifndef __NR_kexec_load
#  define __NR_kexec_load (__NR_SYSCALL_BASE+347)
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat (__NR_SYSCALL_BASE+348)
# endif
# ifndef __NR_signalfd
#  define __NR_signalfd (__NR_SYSCALL_BASE+349)
# endif
# ifndef __NR_timerfd
#  define __NR_timerfd (__NR_SYSCALL_BASE+350)
# endif
# ifndef __NR_eventfd
#  define __NR_eventfd (__NR_SYSCALL_BASE+351)
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
# ifndef __NR_timer_create
#  define __NR_timer_create 259
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime (__NR_timer_create + 1)
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete (__NR_timer_create + 4)
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime (__NR_timer_create + 5)
# endif
# ifndef __NR_clock_gettime
#  define __NR_clock_gettime (__NR_timer_create + 6)
# endif
# ifndef __NR_openat
#  define __NR_openat 295
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
#  define __NR_splice 313
# endif
# ifndef __NR_tee
#  define __NR_tee 315
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 316
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 320
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 322
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
#endif


#ifdef __ia64__
# ifndef __NR_openat
#  define __NR_openat 1281
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
#  define __NR_splice 1297
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
# ifndef __NR_utimensat
#  define __NR_utimensat 1306
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
# ifndef __NR_sync_file_range2
# define __NR_sync_file_range2 308
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 309
# endif
#endif


#ifdef __powerpc__
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
# ifndef __NR_sync_file_range2
# define __NR_sync_file_range2 308
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 309
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
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __s390__
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
# ifndef __NR_splice
#  define __NR_splice 306
# endif
# ifndef __NR_tee
#  define __NR_tee 308
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 309
# endif
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __sparc64__
# ifndef __NR_vmsplice
#  define __NR_vmsplice 25
# endif
# ifndef __NR_splice
#  define __NR_splice 232
# endif
# ifndef __NR_tee
#  define __NR_tee 280
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
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __sparc__
# ifndef __NR_vmsplice
#  define __NR_vmsplice 25
# endif
# ifndef __NR_splice
#  define __NR_splice 232
# endif
# ifndef __NR_tee
#  define __NR_tee 280
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
# ifndef __NR_fallocate
#  define __NR_fallocate 314
# endif
#endif


#ifdef __x86_64__
# ifndef __NR_timer_create
#  define __NR_timer_create 222
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 223
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
# ifndef __NR_openat
#  define __NR_openat 257
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
#  define __NR_splice 275
# endif
# ifndef __NR_tee
#  define __NR_tee 276
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 278
# endif
# ifndef __NR_utimensat
#  define __NR_utimensat 280
# endif
# ifndef __NR_timerfd_create
#  define __NR_timerfd_create 283
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
#endif


# ifndef __NR_clock_gettime
#  define __NR_clock_gettime 0
# endif
# ifndef __NR_clock_settime
#  define __NR_clock_settime 0
# endif
# ifndef __NR_faccessat
#  define __NR_faccessat 0
# endif
# ifndef __NR_fchmodat
#  define __NR_fchmodat 0
# endif
# ifndef __NR_fchownat
#  define __NR_fchownat 0
# endif
# ifndef __NR_fremovexattr
#  define __NR_fremovexattr 0
# endif
# ifndef __NR_fstatat64
#  define __NR_fstatat64 0
# endif
# ifndef __NR_futimesat
#  define __NR_futimesat 0
# endif
# ifndef __NR_linkat
#  define __NR_linkat 0
# endif
# ifndef __NR_mkdirat
#  define __NR_mkdirat 0
# endif
# ifndef __NR_mknodat
#  define __NR_mknodat 0
# endif
# ifndef __NR_newfstatat
#  define __NR_newfstatat 0
# endif
# ifndef __NR_openat
#  define __NR_openat 0
# endif
# ifndef __NR_readlinkat
#  define __NR_readlinkat 0
# endif
# ifndef __NR_renameat
#  define __NR_renameat 0
# endif
# ifndef __NR_splice
#  define __NR_splice 0
# endif
# ifndef __NR_symlinkat
#  define __NR_symlinkat 0
# endif
# ifndef __NR_tee
#  define __NR_tee 0
# endif
# ifndef __NR_timer_create
#  define __NR_timer_create 0
# endif
# ifndef __NR_timer_delete
#  define __NR_timer_delete 0
# endif
# ifndef __NR_timer_settime
#  define __NR_timer_settime 0
# endif
# ifndef __NR_unlinkat
#  define __NR_unlinkat 0
# endif
# ifndef __NR_vmsplice
#  define __NR_vmsplice 0
# endif


#endif
