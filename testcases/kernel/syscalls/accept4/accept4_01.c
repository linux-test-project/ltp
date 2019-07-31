// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2008, Linux Foundation,
 * written by Michael Kerrisk <mtk.manpages@gmail.com>
 * Initial Porting to LTP by Subrata <subrata@linux.vnet.ibm.com>
 *
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define PORT_NUM 33333

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC    O_CLOEXEC
#endif
#ifndef SOCK_NONBLOCK
# define SOCK_NONBLOCK   O_NONBLOCK
#endif

#if defined(SYS_ACCEPT4)	/* the socketcall() number */
#define USE_SOCKETCALL 1
#endif

static struct sockaddr_in *conn_addr, *accept_addr;
static int listening_fd;

#if !(__GLIBC_PREREQ(2, 10))
static int
accept4_01(int fd, struct sockaddr *sockaddr, socklen_t *addrlen, int flags)
{
#if USE_SOCKETCALL
	long args[6];

	args[0] = fd;
	args[1] = (long)sockaddr;
	args[2] = (long)addrlen;
	args[3] = flags;

	return tst_syscall(__NR_socketcall, SYS_ACCEPT4, args);
#else
	return tst_syscall(__NR_accept4, fd, sockaddr, addrlen, flags);
#endif
}
#endif

static int create_listening_socket(void)
{
	struct sockaddr_in svaddr;
	int lfd;
	int optval;

	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svaddr.sin_port = htons(PORT_NUM);

	lfd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);

	optval = 1;
	SAFE_SETSOCKOPT(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	SAFE_BIND(lfd, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in));
	SAFE_LISTEN(lfd, 5);

	return lfd;
}

static void setup(void)
{
	memset(conn_addr, 0, sizeof(*conn_addr));
	conn_addr->sin_family = AF_INET;
	conn_addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	conn_addr->sin_port = htons(PORT_NUM);

	listening_fd = create_listening_socket();
}

static void cleanup(void)
{
	SAFE_CLOSE(listening_fd);
}

static struct test_case {
	int cloexec;
	int nonblock;
} tcases[] = {
	{ 0, 0 },
	{ SOCK_CLOEXEC, 0 },
	{ 0, SOCK_NONBLOCK },
	{ SOCK_CLOEXEC, SOCK_NONBLOCK },
};

static void verify_accept4(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];
	int connfd, acceptfd;
	int fdf, flf, fdf_pass, flf_pass, fd_cloexec, fd_nonblock;
	socklen_t addrlen;

	connfd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_CONNECT(connfd, (struct sockaddr *)conn_addr, sizeof(*conn_addr));
	addrlen = sizeof(*accept_addr);

#if !(__GLIBC_PREREQ(2, 10))
	TEST(accept4_01(listening_fd, (struct sockaddr *)accept_addr, &addrlen,
				tcase->cloexec | tcase->nonblock));
#else
	TEST(accept4(listening_fd, (struct sockaddr *)accept_addr, &addrlen,
				tcase->cloexec | tcase->nonblock));
#endif
	if (TST_RET == -1) {
		if (TST_ERR == ENOSYS)
			tst_brk(TCONF, "syscall __NR_accept4 not supported");
		else
			tst_brk(TBROK | TTERRNO, "accept4 failed");
	}

	acceptfd = TST_RET;

	/* Test to see if O_CLOEXEC is as expected */
	fdf = SAFE_FCNTL(acceptfd, F_GETFD);
	fd_cloexec = !!(fdf & FD_CLOEXEC);
	fdf_pass = fd_cloexec == !!tcase->cloexec;
	if (!fdf_pass) {
		tst_res(TFAIL, "Close-on-exec flag mismatch, %d vs %d",
			fd_cloexec, !!tcase->cloexec);
	}

	/* Test to see if O_NONBLOCK is as expected */
	flf = SAFE_FCNTL(acceptfd, F_GETFL);
	fd_nonblock = !!(flf & O_NONBLOCK);
	flf_pass = fd_nonblock == !!tcase->nonblock;
	if (!flf_pass) {
		tst_res(TFAIL, "nonblock flag mismatch, %d vs %d",
		        fd_nonblock, !!tcase->nonblock);
	}

	SAFE_CLOSE(acceptfd);
	SAFE_CLOSE(connfd);

	if (fdf_pass && flf_pass) {
		tst_res(TPASS, "Close-on-exec %d, nonblock %d",
				fd_cloexec, fd_nonblock);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_accept4,
	.bufs = (struct tst_buffers []) {
		{&conn_addr, .size = sizeof(*conn_addr)},
		{&accept_addr, .size = sizeof(*accept_addr)},
		{},
	}
};
