// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 */

#ifndef SENDMMSG_VAR__
#define SENDMMSG_VAR__

#include "tst_timer.h"
#include "lapi/syscalls.h"

static inline int libc_sendmmsg(int sockfd, struct mmsghdr *msgvec,
				unsigned int vlen, unsigned int flags)
{
#ifdef HAVE_SENDMMSG
	return sendmmsg(sockfd, msgvec, vlen, flags);
#else
	tst_brk(TCONF, "libc sendmmsg not present");
	return -1;
#endif
}

static inline int sys_sendmmsg(int sockfd, struct mmsghdr *msgvec,
			       unsigned int vlen, unsigned int flags)
{
	return tst_syscall(__NR_sendmmsg, sockfd, msgvec, vlen, flags);
}

static inline int libc_recvmmsg(int sockfd, struct mmsghdr *msgvec,
			unsigned int vlen, unsigned int flags, void *timeout)
{
#ifdef HAVE_RECVMMSG
	return recvmmsg(sockfd, msgvec, vlen, flags, timeout);
#else
	tst_brk(TCONF, "libc recvmmsg not present");
	return -1;
#endif
}

static inline int sys_recvmmsg(int sockfd, struct mmsghdr *msgvec,
			unsigned int vlen, unsigned int flags, void *timeout)
{
	return tst_syscall(__NR_recvmmsg, sockfd, msgvec, vlen, flags, timeout);
}

static inline int sys_recvmmsg64(int sockfd, struct mmsghdr *msgvec,
			unsigned int vlen, unsigned int flags, void *timeout)
{
	return tst_syscall(__NR_recvmmsg_time64, sockfd, msgvec, vlen, flags,
			   timeout);
}

#endif /* SENDMMSG_VAR__ */
