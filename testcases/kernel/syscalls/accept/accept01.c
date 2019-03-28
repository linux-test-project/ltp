// SPDX-License-Identifier: GPL-2.0-or-later

/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 *
 *   Description:
 *     Verify that accept() returns the proper errno for various failure cases
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>

#include <netinet/in.h>

#include "tst_test.h"

struct sockaddr_in sin0, sin1, fsin1;

int invalid_socketfd = 400; /* anything that is not an open file */
int devnull_fd;
int socket_fd;
int udp_fd;

static struct test_case {
	int domain;		/* PF_INET, PF_UNIX, ... */
	int type;		/* SOCK_STREAM, SOCK_DGRAM ... */
	int proto;		/* protocol number (usually 0 = default) */
	int *fd;		/* File descriptor for the test case */
	struct sockaddr *sockaddr;	/* socket address buffer */
	socklen_t salen;	/* accept's 3rd argument */
	int experrno;		/* expected errno */
	char *desc;
} tcases[] = {
	{
		PF_INET, SOCK_STREAM, 0, &invalid_socketfd,
		(struct sockaddr *)&fsin1, sizeof(fsin1), EBADF,
		"bad file descriptor"
	},
	{
		PF_INET, SOCK_STREAM, 0, &devnull_fd, (struct sockaddr *)&fsin1,
		sizeof(fsin1), ENOTSOCK, "fd is not socket"
	},
	{
		PF_INET, SOCK_STREAM, 0, &socket_fd, (struct sockaddr *)3,
		sizeof(fsin1), EINVAL, "invalid socket buffer"
	},
	{
		PF_INET, SOCK_STREAM, 0, &socket_fd, (struct sockaddr *)&fsin1,
		1, EINVAL, "invalid salen"
	},
	{
		PF_INET, SOCK_STREAM, 0, &socket_fd, (struct sockaddr *)&fsin1,
		sizeof(fsin1), EINVAL, "no queued connections"
	},
	{
		PF_INET, SOCK_STREAM, 0, &udp_fd, (struct sockaddr *)&fsin1,
		sizeof(fsin1), EOPNOTSUPP, "UDP accept"
	},
};

static void test_setup(void)
{
	sin0.sin_family = AF_INET;
	sin0.sin_port = 0;
	sin0.sin_addr.s_addr = INADDR_ANY;

	devnull_fd = SAFE_OPEN("/dev/null", O_WRONLY);

	socket_fd = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	SAFE_BIND(socket_fd, (struct sockaddr *)&sin0, sizeof(sin0));

	sin1.sin_family = AF_INET;
	sin1.sin_port = 0;
	sin1.sin_addr.s_addr = INADDR_ANY;

	udp_fd = SAFE_SOCKET(PF_INET, SOCK_DGRAM, 0);
	SAFE_BIND(udp_fd, (struct sockaddr *)&sin1, sizeof(sin1));
}

static void test_cleanup(void)
{
	SAFE_CLOSE(devnull_fd);
	SAFE_CLOSE(socket_fd);
	SAFE_CLOSE(udp_fd);
}

void verify_accept(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	TEST(accept(*tcase->fd, tcase->sockaddr, &tcase->salen));

	if (TST_RET != -1) {
		tst_res(TFAIL, "%s: returned %li, expected -1",
				tcase->desc, TST_RET);
		return;
	}

	if (TST_ERR != tcase->experrno) {
		tst_res(TFAIL | TTERRNO, "%s: expected errno %s, got ",
				tcase->desc, tst_strerrno(tcase->experrno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s successful", tcase->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = test_setup,
	.cleanup = test_cleanup,
	.test = verify_accept,
};
