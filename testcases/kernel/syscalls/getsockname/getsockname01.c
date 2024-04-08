// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 * Verify that getsockname() returns the proper errno for various failure cases:
 * - EBADF on a not open file
 * - ENOTSOCK on a file descriptor not linked to a socket
 * - EFAULT on invalid socket buffer o invalid socklen
 */

#include "tst_test.h"

static struct sockaddr_in sin0, fsin1;
static int sock_null, sock_bind, sock_fake;
static socklen_t sinlen;

static struct test_case {
	int *sock;
	struct sockaddr_in *sockaddr;
	socklen_t *addrlen;
	int experrno;
	char *desc;
} tcases[] = {
	{ .sock = &sock_fake, .sockaddr = &fsin1, .addrlen = &sinlen,
		.experrno = EBADF, "bad file descriptor"},
	{ .sock = &sock_null, .sockaddr = &fsin1, .addrlen = &sinlen,
		.experrno = ENOTSOCK, "bad file descriptor"},
	{ .sock = &sock_bind, .sockaddr = NULL, .addrlen = &sinlen,
		.experrno = EFAULT, "invalid socket buffer"},
	{ .sock = &sock_bind, .sockaddr = &fsin1, .addrlen = NULL,
		.experrno = EFAULT, "invalid aligned salen"},
	{ .sock = &sock_bind, .sockaddr = &fsin1, .addrlen = (socklen_t *) 1,
		.experrno = EFAULT, "invalid unaligned salen"},
};

static void check_getsockname(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	TST_EXP_FAIL(getsockname(*(tc->sock), (struct sockaddr *) tc->sockaddr,
				 tc->addrlen), tc->experrno, "%s", tc->desc);
}

static void setup(void)
{
	sin0.sin_family = AF_INET;
	sin0.sin_port = 0;
	sin0.sin_addr.s_addr = INADDR_ANY;
	sock_fake = 400;
	sock_null = SAFE_OPEN("/dev/null", O_WRONLY);
	sock_bind = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	SAFE_BIND(sock_bind, (struct sockaddr *)&sin0, sizeof(sin0));
	sinlen = sizeof(sin0);
}

static struct tst_test test = {
	.setup = setup,
	.test = check_getsockname,
	.tcnt = ARRAY_SIZE(tcases),
};
