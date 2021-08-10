// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_SEMBUF_H__
#define LAPI_SEMBUF_H__

#include "lapi/posix_types.h"
#include <sys/sem.h>
#include "tst_timer.h"
#include "ipcbuf.h"

#ifndef HAVE_SEMID64_DS

#if defined(__mips__)
#define HAVE_SEMID64_DS
/*
 * The semid64_ds structure for the MIPS architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for 2 miscellaneous 64-bit values on mips64,
 * but used for the upper 32 bit of the time values on mips32.
 */
#if __BITS_PER_LONG == 64
struct semid64_ds {
	struct ipc64_perm sem_perm;		/* permissions .. see ipc.h */
	long		 sem_otime;		/* last semop time */
	long		 sem_ctime;		/* last change time */
	unsigned long	sem_nsems;		/* no. of semaphores in array */
	unsigned long	__unused1;
	unsigned long	__unused2;
};
#else
#define HAVE_SEMID64_DS_TIME_HIGH
struct semid64_ds {
	struct ipc64_perm sem_perm;		/* permissions .. see ipc.h */
	unsigned long   sem_otime;		/* last semop time */
	unsigned long   sem_ctime;		/* last change time */
	unsigned long	sem_nsems;		/* no. of semaphores in array */
	unsigned long	sem_otime_high;
	unsigned long	sem_ctime_high;
};
#endif
#endif /* __mips__ */

#if defined(__hppa__)
#define HAVE_SEMID64_DS
/*
 * The semid64_ds structure for parisc architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */
struct semid64_ds {
	struct ipc64_perm sem_perm;		/* permissions .. see ipc.h */
#if __BITS_PER_LONG == 64
	long		sem_otime;		/* last semop time */
	long		sem_ctime;		/* last change time */
#else
#define HAVE_SEMID64_DS_TIME_HIGH
	unsigned long	sem_otime_high;
	unsigned long	sem_otime;		/* last semop time */
	unsigned long	sem_ctime_high;
	unsigned long	sem_ctime;		/* last change time */
#endif
	unsigned long	sem_nsems;		/* no. of semaphores in array */
	unsigned long	__unused1;
	unsigned long	__unused2;
};
#endif /* __hppa__ */

#if defined(__powerpc__) || defined(__powerpc64__)
#define HAVE_SEMID64_DS
/*
 * The semid64_ds structure for PPC architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32/64-bit values
 */

struct semid64_ds {
	struct ipc64_perm sem_perm;	/* permissions .. see ipc.h */
#ifndef __powerpc64__
#define HAVE_SEMID64_DS_TIME_HIGH
	unsigned long	sem_otime_high;
	unsigned long	sem_otime;	/* last semop time */
	unsigned long	sem_ctime_high;
	unsigned long	sem_ctime;	/* last change time */
#else
	long		sem_otime;	/* last semop time */
	long		sem_ctime;	/* last change time */
#endif
	unsigned long	sem_nsems;	/* no. of semaphores in array */
	unsigned long	__unused3;
	unsigned long	__unused4;
};
#endif /* defined(__powerpc__) || defined(__powerpc64__) */

#if defined(__sparc__)
#define HAVE_SEMID64_DS
/*
 * The semid64_ds structure for sparc architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct semid64_ds {
	struct ipc64_perm sem_perm;		/* permissions .. see ipc.h */
#if defined(__arch64__)
	long		sem_otime;		/* last semop time */
	long		sem_ctime;		/* last change time */
#else
#define HAVE_SEMID64_DS_TIME_HIGH
	unsigned long	sem_otime_high;
	unsigned long	sem_otime;		/* last semop time */
	unsigned long	sem_ctime_high;
	unsigned long	sem_ctime;		/* last change time */
#endif
	unsigned long	sem_nsems;		/* no. of semaphores in array */
	unsigned long	__unused1;
	unsigned long	__unused2;
};
#endif /* __sparc__ */

#if defined(__x86_64__)
#define HAVE_SEMID64_DS
/*
 * The semid64_ds structure for x86 architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 *
 * x86_64 and x32 incorrectly added padding here, so the structures
 * are still incompatible with the padding on x86.
 */
struct semid64_ds {
	struct ipc64_perm sem_perm;	/* permissions .. see ipc.h */
#ifdef __i386__
#define HAVE_SEMID64_DS_TIME_HIGH
	unsigned long	sem_otime;	/* last semop time */
	unsigned long	sem_otime_high;
	unsigned long	sem_ctime;	/* last change time */
	unsigned long	sem_ctime_high;
#else
	__kernel_long_t sem_otime;	/* last semop time */
	__kernel_ulong_t __unused1;
	__kernel_long_t sem_ctime;	/* last change time */
	__kernel_ulong_t __unused2;
#endif
	__kernel_ulong_t sem_nsems;	/* no. of semaphores in array */
	__kernel_ulong_t __unused3;
	__kernel_ulong_t __unused4;
};
#endif /* defined(__x86_64__) */

#if defined(__xtensa__)
#define HAVE_SEMID64_DS
#define HAVE_SEMID64_DS_TIME_HIGH

struct semid64_ds {
	struct ipc64_perm sem_perm;		/* permissions .. see ipc.h */
#ifdef __XTENSA_EL__
	unsigned long	sem_otime;		/* last semop time */
	unsigned long	sem_otime_high;
	unsigned long	sem_ctime;		/* last change time */
	unsigned long	sem_ctime_high;
#else
	unsigned long	sem_otime_high;
	unsigned long	sem_otime;		/* last semop time */
	unsigned long	sem_ctime_high;
	unsigned long	sem_ctime;		/* last change time */
#endif
	unsigned long	sem_nsems;		/* no. of semaphores in array */
	unsigned long	__unused3;
	unsigned long	__unused4;
};

#endif /* __xtensa__ */

#ifndef HAVE_SEMID64_DS
/*
 * The semid64_ds structure for most architectures (though it came
 * from x86_32 originally). Note extra padding because this structure
 * is passed back and forth between kernel and user space.
 *
 * semid64_ds was originally meant to be architecture specific, but
 * everyone just ended up making identical copies without specific
 * optimizations, so we may just as well all use the same one.
 *
 * 64 bit architectures use a 64-bit long time field here, while
 * 32 bit architectures have a pair of unsigned long values.
 *
 * On big-endian systems, the padding is in the wrong place for
 * historic reasons, so user space has to reconstruct a time_t
 * value using
 *
 * user_semid_ds.sem_otime = kernel_semid64_ds.sem_otime +
 *		((long long)kernel_semid64_ds.sem_otime_high << 32)
 *
 * Pad space is left for 2 miscellaneous 32-bit values
 */
struct semid64_ds {
	struct ipc64_perm sem_perm;	/* permissions .. see ipc.h */
#if __BITS_PER_LONG == 64
	long		sem_otime;	/* last semop time */
	long		sem_ctime;	/* last change time */
#else
#define HAVE_SEMID64_DS_TIME_HIGH
	unsigned long	sem_otime;	/* last semop time */
	unsigned long	sem_otime_high;
	unsigned long	sem_ctime;	/* last change time */
	unsigned long	sem_ctime_high;
#endif
	unsigned long	sem_nsems;	/* no. of semaphores in array */
	unsigned long	__unused3;
	unsigned long	__unused4;
};
#endif /* semid64_ds */

#endif /* HAVE_SEMID64_DS */

#endif /* LAPI_SEMBUF_H__ */
