// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Author: John George
 * Copyright (c) 2020 Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * Verify that setsockopt() fails and set errno:
 *
 * - EBADF on invalid file descriptor
 * - ENOTSOCK on non-socket file descriptor
 * - EFAULT on invalid option buffer
 * - EINVAL on invalid optlen
 * - ENOPROTOOPT on invalid level
 * - ENOPROTOOPT on invalid option name (UDP)
 * - ENOPROTOOPT on invalid option name (IP)
 * - ENOPROTOOPT on invalid option name (TCP)
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include "tst_test.h"

static struct sockaddr_in addr;
static int optval;

static struct test_case {	/* test case structure */
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	int level;		/* IPPROTO_* */
	int optname;
	void *optval;
	int optlen;
	int experrno;		/* expected errno */
	char *desc;
} testcase_list[] = {
	{-1, -1, -1, SOL_SOCKET, SO_OOBINLINE, &optval, sizeof(optval),
		EBADF, "invalid file descriptor"},
	{-1, -1, -1, SOL_SOCKET, SO_OOBINLINE, &optval, sizeof(optval),
		ENOTSOCK, "non-socket file descriptor"},
	{PF_INET, SOCK_STREAM, 0, SOL_SOCKET, SO_OOBINLINE, NULL,
		sizeof(optval), EFAULT, "invalid option buffer"},
	{PF_INET, SOCK_STREAM, 0, SOL_SOCKET, SO_OOBINLINE, &optval, 0,
		EINVAL, "invalid optlen"},
	{PF_INET, SOCK_STREAM, 0, 500, SO_OOBINLINE, &optval, sizeof(optval),
		ENOPROTOOPT, "invalid level"},
	{PF_INET, SOCK_STREAM, 0, IPPROTO_UDP, SO_OOBINLINE, &optval,
		sizeof(optval), ENOPROTOOPT, "invalid option name (UDP)"},
	{PF_INET, SOCK_STREAM, 0, IPPROTO_IP, -1, &optval, sizeof(optval),
		ENOPROTOOPT, "invalid option name (IP)"},
	{PF_INET, SOCK_STREAM, 0, IPPROTO_TCP, -1, &optval, sizeof(optval),
		ENOPROTOOPT, "invalid option name (TCP)"}
};

static void setup(void)
{
	/* initialize local sockaddr */
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = INADDR_ANY;
}

static void run(unsigned int n)
{
	struct test_case *tc = testcase_list + n;
	int tmpfd, fd;

	tst_res(TINFO, "Testing %s", tc->desc);

	if (tc->domain == -1) {
		tmpfd = fd = SAFE_OPEN("/dev/null", O_WRONLY);
	} else {
		tmpfd = fd = SAFE_SOCKET(tc->domain, tc->type, tc->proto);
		SAFE_BIND(fd, (struct sockaddr *)&addr, sizeof(addr));
	}

	/* Use closed file descriptor rather than -1 */
	if (tc->experrno == EBADF)
		SAFE_CLOSE(tmpfd);

	TEST(setsockopt(fd, tc->level, tc->optname, tc->optval, tc->optlen));

	if (tc->experrno != EBADF)
		SAFE_CLOSE(fd);

	if (TST_RET == 0) {
		tst_res(TFAIL, "setsockopt() succeeded unexpectedly");
		return;
	}

	if (TST_RET != -1) {
		tst_res(TFAIL | TTERRNO,
			"Invalid setsockopt() return value %ld", TST_RET);
		return;
	}

	if (TST_ERR != tc->experrno) {
		tst_res(TFAIL | TTERRNO,
			"setsockopt() returned unexpected error");
		return;
	}

	tst_res(TPASS | TTERRNO, "setsockopt() returned the expected error");
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.setup = setup
};
