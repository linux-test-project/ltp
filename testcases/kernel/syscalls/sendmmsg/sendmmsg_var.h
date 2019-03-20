// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 */

#ifndef SENDMMSG_VAR__
#define SENDMMSG_VAR__

#include "lapi/syscalls.h"

static int do_sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
		       int flags)
{
	switch (tst_variant) {
	case 0:
		return tst_syscall(__NR_sendmmsg, sockfd, msgvec, vlen, flags);
	case 1:
#ifdef HAVE_SENDMMSG
		return sendmmsg(sockfd, msgvec, vlen, flags);
#else
		tst_brk(TCONF, "libc sendmmsg not present");
#endif
	}

	return -1;
}

static int do_recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
		       int flags, struct timespec *timeout)
{
	switch (tst_variant) {
	case 0:
		return tst_syscall(__NR_recvmmsg, sockfd, msgvec, vlen, flags,
				   timeout);
	case 1:
#ifdef HAVE_RECVMMSG
		return recvmmsg(sockfd, msgvec, vlen, flags, timeout);
#else
		tst_brk(TCONF, "libc recvmmsg not present");
#endif
	}

	return -1;
}

static void test_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing direct sendmmsg and recvmmsg syscalls");
		break;
	case 1:
		tst_res(TINFO, "Testing libc sendmmsg and recvmmsg syscalls");
		break;
	}
}

#define TEST_VARIANTS 2

#endif /* SENDMMSG_VAR__ */
