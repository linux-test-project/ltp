// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) Linux Test Project, 2003-2017
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 * Verify that getsockopt() returns the proper errno for various failure cases:
 *
 * - EBADF on a not open file
 * - ENOTSOCK on a file descriptor not linked to a socket
 * - EFAULT on invalid address of value or length
 * - EOPNOTSUPP on invalid option name or protocol
 * - EINVAL on an invalid optlen
 */

#include "tst_test.h"

static int sock_fake, sock_null, sock_bind;
static struct sockaddr_in sin0;
static int sinlen;
static int optval;
static socklen_t optlen;
static socklen_t optleninval = -1;

static struct test_case {
	int *sockfd;
	int level;
	int optname;
	void *optval;
	socklen_t *optlen;
	int experrno;
	char *desc;
} tcases[] = {
	{.sockfd = &sock_fake, .level = SOL_SOCKET, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = &optlen, .experrno = EBADF, .desc = "bad file descriptor"},

	{.sockfd = &sock_null, .level = SOL_SOCKET, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = &optlen, .experrno = ENOTSOCK, .desc = "bad file descriptor"},

	{.sockfd = &sock_bind, .level = SOL_SOCKET, .optname = SO_OOBINLINE, .optval = 0,
	.optlen = &optlen, .experrno = EFAULT, .desc = "invalid option buffer"},

	{.sockfd = &sock_bind, .level = SOL_SOCKET, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = 0, .experrno = EFAULT, .desc = "invalid optlen"},

	{.sockfd = &sock_bind, .level = 500, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = &optlen, .experrno = EOPNOTSUPP, .desc = "invalid level"},

	{.sockfd = &sock_bind, .level = IPPROTO_UDP, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = &optlen, .experrno = EOPNOTSUPP, .desc = "not supported option name (UDP)"},

	{.sockfd = &sock_bind, .level = IPPROTO_IP, .optname = -1, .optval = &optval,
	.optlen = &optlen, .experrno = ENOPROTOOPT, .desc =  "invalid option name (IP)"},

	{.sockfd = &sock_bind, .level = IPPROTO_TCP, .optname = -1, .optval = &optval,
	.optlen = &optlen, .experrno = ENOPROTOOPT, .desc = "invalid option name (TCP)"},

	{.sockfd = &sock_bind, .level = SOL_SOCKET, .optname = SO_OOBINLINE, .optval = &optval,
	.optlen = &optleninval, .experrno = EINVAL, .desc = "invalid optlen"},
};


static void check_getsockopt(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	TST_EXP_FAIL(getsockopt(*(tc->sockfd), tc->level, tc->optname, tc->optval, tc->optlen),
		     tc->experrno, "%s", tc->desc);
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
	optlen = sizeof(optval);
}

static struct tst_test test = {
	.setup = setup,
	.test = check_getsockopt,
	.tcnt = ARRAY_SIZE(tcases),
};
