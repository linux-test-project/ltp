/*
 * Here we stick all the ugly *fallback* logic for linux
 * system call numbers (those __NR_ thingies).
 *
 * Licensed under the GPLv2 or later, see the COPYING file.
 */

#ifndef __LINUX_SYSCALL_NUMBERS_H__
#define __LINUX_SYSCALL_NUMBERS_H__

#include <sys/syscall.h>


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
#endif


#ifdef __parisc__
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
