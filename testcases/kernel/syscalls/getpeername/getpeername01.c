// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 *
 */

/*\
 * [Description]
 * Verify that getpeername() returns the proper errno for various failure cases:
 * - EBADF on invalid address.
 * - ENOTSOCK on socket opened on /dev/null.
 * - ENOTCONN on socket not connected.
 * - EINVAL on negative addrlen.
 * - EFAULT on invalid addr/addrlen pointers.
 */

#include "tst_test.h"

static struct sockaddr_in server_addr;
static struct sockaddr_in fsin1;
static socklen_t sinlen;
static socklen_t invalid_sinlen = -1;
static int sv[2];
static int sockfd = -1;

static void setup_fd_file(void)
{
	sockfd = SAFE_OPEN("/dev/null", O_WRONLY, 0666);
}

static void setup_fd_stream(void)
{
	sockfd = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	SAFE_BIND(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
}

static void cleanup_fd(void)
{
	if (sockfd)
		SAFE_CLOSE(sockfd);
}

static void setup_pair(void)
{
	SAFE_SOCKETPAIR(PF_UNIX, SOCK_STREAM, 0, sv);
	sockfd = sv[0];
}

static void cleanup_pair(void)
{
	if (sv[0])
		SAFE_CLOSE(sv[0]);
	if (sv[1])
		SAFE_CLOSE(sv[1]);
}

static struct test_case {
	struct sockaddr *sockaddr;
	socklen_t *addrlen;
	int experrno;
	void (*case_setup)(void);
	void (*case_cleanup)(void);
} test_cases[] = {
	{.addrlen = &sinlen, .experrno = EBADF},
	{.addrlen = &sinlen, .experrno = ENOTSOCK, .case_setup = setup_fd_file, .case_cleanup = cleanup_fd },
	{.addrlen = &sinlen, .experrno = ENOTCONN, .case_setup = setup_fd_stream, .case_cleanup = cleanup_fd },
	{.addrlen = &invalid_sinlen, .experrno = EINVAL, .case_setup = setup_pair, .case_cleanup = cleanup_pair},
	{.sockaddr = (struct sockaddr *) -1, .addrlen = &sinlen, .experrno = EFAULT, .case_setup = setup_pair, .case_cleanup = cleanup_pair},
	{.experrno = EFAULT, .case_setup = setup_pair, .case_cleanup = cleanup_pair},
	{.addrlen = (socklen_t *) 1, .experrno = EFAULT, .case_setup = setup_pair, .case_cleanup = cleanup_pair },
};

static void verify_getpeername(unsigned int nr)
{
	struct test_case *tc = &test_cases[nr];

	if (tc->case_setup != NULL)
		tc->case_setup();

	if (!tc->sockaddr)
		tc->sockaddr = (struct sockaddr *)&fsin1;

	TST_EXP_FAIL(getpeername(sockfd, tc->sockaddr, tc->addrlen), tc->experrno);

	if (tc->case_cleanup != NULL)
		tc->case_cleanup();
}

static void setup(void)
{
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = 0;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	sinlen = sizeof(fsin1);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_getpeername,
	.tcnt = ARRAY_SIZE(test_cases),
};
