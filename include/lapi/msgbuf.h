// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_MSGBUF_H__
#define LAPI_MSGBUF_H__

#include "lapi/posix_types.h"
#include <sys/sem.h>
#include "tst_timer.h"
#include "ipcbuf.h"

#ifndef HAVE_MSQID64_DS

#if defined(__mips__)
#define HAVE_MSQID64_DS

#if __BITS_PER_LONG == 64
/*
 * The msqid64_ds structure for the MIPS architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous unsigned long values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
	long msg_stime;			/* last msgsnd time */
	long msg_rtime;			/* last msgrcv time */
	long msg_ctime;			/* last change time */
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
};
#elif defined (__MIPSEB__)
#define HAVE_MSQID64_DS_TIME_HIGH
struct msqid64_ds {
	struct ipc64_perm msg_perm;
	unsigned long  msg_stime_high;
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_ctime_high;
	unsigned long  msg_ctime;	/* last change time */
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
};
#elif defined (__MIPSEL__)
#define HAVE_MSQID64_DS_TIME_HIGH
struct msqid64_ds {
	struct ipc64_perm msg_perm;
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_stime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_ctime;	/* last change time */
	unsigned long  msg_ctime_high;
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
};
#endif

#endif /* __mips__ */

#if defined(__hppa__)
#define HAVE_MSQID64_DS
/*
 * The msqid64_ds structure for parisc architecture, copied from sparc.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#if __BITS_PER_LONG == 64
	long		 msg_stime;	/* last msgsnd time */
	long		 msg_rtime;	/* last msgrcv time */
	long		 msg_ctime;	/* last change time */
#else
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long	msg_stime_high;
	unsigned long	msg_stime;	/* last msgsnd time */
	unsigned long	msg_rtime_high;
	unsigned long	msg_rtime;	/* last msgrcv time */
	unsigned long	msg_ctime_high;
	unsigned long	msg_ctime;	/* last change time */
#endif
	unsigned long	msg_cbytes;	/* current number of bytes on queue */
	unsigned long	msg_qnum;	/* number of messages in queue */
	unsigned long	msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t	msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t	msg_lrpid;	/* last receive pid */
	unsigned long	__unused1;
	unsigned long	__unused2;
};

#endif /* __hppa__ */

#if defined(__powerpc__) || defined(__powerpc64__)
#define HAVE_MSQID64_DS
/*
 * The msqid64_ds structure for the PowerPC architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#ifdef __powerpc64__
	long		 msg_stime;	/* last msgsnd time */
	long		 msg_rtime;	/* last msgrcv time */
	long		 msg_ctime;	/* last change time */
#else
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long  msg_stime_high;
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_ctime_high;
	unsigned long  msg_ctime;	/* last change time */
#endif
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
};

#endif /* defined(__powerpc__) || defined(__powerpc64__) */

#if defined(__sparc__)
#define HAVE_MSQID64_DS
/*
 * The msqid64_ds structure for sparc64 architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */
struct msqid64_ds {
	struct ipc64_perm msg_perm;
#if defined(__arch64__)
	long msg_stime;			/* last msgsnd time */
	long msg_rtime;			/* last msgrcv time */
	long msg_ctime;			/* last change time */
#else
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long msg_stime_high;
	unsigned long msg_stime;	/* last msgsnd time */
	unsigned long msg_rtime_high;
	unsigned long msg_rtime;	/* last msgrcv time */
	unsigned long msg_ctime_high;
	unsigned long msg_ctime;	/* last change time */
#endif
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused1;
	unsigned long  __unused2;
};

#endif /* __sparc__ */

#if defined(__x86_64__) && defined(__ILP32__)
#define HAVE_MSQID64_DS
/*
 * The msqid64_ds structure for x86 architecture with x32 ABI.
 *
 * On x86-32 and x86-64 we can just use the generic definition, but
 * x32 uses the same binary layout as x86_64, which is differnet
 * from other 32-bit architectures.
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
	__kernel_long_t msg_stime;	/* last msgsnd time */
	__kernel_long_t msg_rtime;	/* last msgrcv time */
	__kernel_long_t msg_ctime;	/* last change time */
	__kernel_ulong_t msg_cbytes;	/* current number of bytes on queue */
	__kernel_ulong_t msg_qnum;	/* number of messages in queue */
	__kernel_ulong_t msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	__kernel_ulong_t __unused4;
	__kernel_ulong_t __unused5;
};

#endif /* defined(__x86_64__) && defined(__ILP32__) */

#if defined(__xtensa__)
#define HAVE_MSQID64_DS
/*
 * The msqid64_ds structure for the Xtensa architecture.
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#ifdef __XTENSA_EB__
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long  msg_stime_high;
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_ctime_high;
	unsigned long  msg_ctime;	/* last change time */
#elif defined(__XTENSA_EL__)
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long  msg_stime;	/* last msgsnd time */
	unsigned long  msg_stime_high;
	unsigned long  msg_rtime;	/* last msgrcv time */
	unsigned long  msg_rtime_high;
	unsigned long  msg_ctime;	/* last change time */
	unsigned long  msg_ctime_high;
#else
# error processor byte order undefined!
#endif
	unsigned long  msg_cbytes;	/* current number of bytes on queue */
	unsigned long  msg_qnum;	/* number of messages in queue */
	unsigned long  msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long  __unused4;
	unsigned long  __unused5;
};

#endif /* __xtensa__ */

#ifndef HAVE_MSQID64_DS
/*
 * generic msqid64_ds structure.
 *
 * Note extra padding because this structure is passed back and forth
 * between kernel and user space.
 *
 * msqid64_ds was originally meant to be architecture specific, but
 * everyone just ended up making identical copies without specific
 * optimizations, so we may just as well all use the same one.
 *
 * 64 bit architectures use a 64-bit long time field here, while
 * 32 bit architectures have a pair of unsigned long values.
 * On big-endian systems, the lower half is in the wrong place.
 *
 * Pad space is left for:
 * - 2 miscellaneous 32-bit values
 */

struct msqid64_ds {
	struct ipc64_perm msg_perm;
#if __BITS_PER_LONG == 64
	long		 msg_stime;	/* last msgsnd time */
	long		 msg_rtime;	/* last msgrcv time */
	long		 msg_ctime;	/* last change time */
#else
#define HAVE_MSQID64_DS_TIME_HIGH
	unsigned long	msg_stime;	/* last msgsnd time */
	unsigned long	msg_stime_high;
	unsigned long	msg_rtime;	/* last msgrcv time */
	unsigned long	msg_rtime_high;
	unsigned long	msg_ctime;	/* last change time */
	unsigned long	msg_ctime_high;
#endif
	unsigned long	msg_cbytes;	/* current number of bytes on queue */
	unsigned long	msg_qnum;	/* number of messages in queue */
	unsigned long	 msg_qbytes;	/* max number of bytes on queue */
	__kernel_pid_t msg_lspid;	/* pid of last msgsnd */
	__kernel_pid_t msg_lrpid;	/* last receive pid */
	unsigned long	 __unused4;
	unsigned long	 __unused5;
};

#endif /* msqid64_ds */

#endif /* HAVE_MSQID64_DS */

#endif /* LAPI_MSGBUF_H__ */
