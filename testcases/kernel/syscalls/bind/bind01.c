// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "tst_test.h"

#define DIR_ENOTDIR "dir_enotdir"
#define TEST_ENOTDIR "test_enotdir"

static int inet_socket;
static int dev_null;
static int fd_ebadf = -1;
static int fd_enotdir;

static struct sockaddr_in sin1, sin2, sin3;
static struct sockaddr_un sun, sock_enotdir;

static struct test_case {
	int *socket_fd;
	struct sockaddr *sockaddr;
	socklen_t salen;
	int retval;
	int experrno;
	char *desc;
} tcases[] = {
	{ &inet_socket, (struct sockaddr *)&sin1, 3, -1,
	  EINVAL, "invalid salen" },
	{ &dev_null, (struct sockaddr *)&sin1, sizeof(sin1), -1,
	  ENOTSOCK, "invalid socket" },
	{ &inet_socket, (struct sockaddr *)&sin2, sizeof(sin2), 0,
	  0, "INADDR_ANYPORT"},
	{ &inet_socket, (struct sockaddr *)&sun, sizeof(sun), -1,
	  EAFNOSUPPORT, "UNIX-domain of current directory" },
	{ &inet_socket, (struct sockaddr *)&sin3, sizeof(sin3), -1,
	  EADDRNOTAVAIL, "non-local address" },
	{ &fd_ebadf, (struct sockaddr *)&sin1, sizeof(sin1), -1,
	  EBADF, "sockfd is not a valid file descriptor" },
	{ &fd_enotdir, (struct sockaddr *)&sock_enotdir, sizeof(sock_enotdir), -1,
	  ENOTDIR, "a component of addr prefix is not a directory"},
};

static void verify_bind(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	if (tcase->experrno) {
		TST_EXP_FAIL(bind(*tcase->socket_fd, tcase->sockaddr, tcase->salen),
				tcase->experrno, "%s", tcase->desc);
	} else {
		TST_EXP_PASS(bind(*tcase->socket_fd, tcase->sockaddr, tcase->salen),
				"%s", tcase->desc);
		SAFE_CLOSE(inet_socket);
		inet_socket = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	}
}

static void test_setup(void)
{
	/* initialize sockaddr's */
	sin1.sin_family = AF_INET;
	/* this port must be unused! */
	sin1.sin_port = TST_GET_UNUSED_PORT(AF_INET, SOCK_STREAM);
	sin1.sin_addr.s_addr = INADDR_ANY;

	sin2.sin_family = AF_INET;
	sin2.sin_port = 0;
	sin2.sin_addr.s_addr = INADDR_ANY;

	sin3.sin_family = AF_INET;
	sin3.sin_port = 0;
	/* assumes 10.255.254.253 is not a local interface address! */
	sin3.sin_addr.s_addr = htonl(0x0AFFFEFD);

	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, ".", sizeof(sun.sun_path));

	SAFE_TOUCH(DIR_ENOTDIR, 0777, NULL);
	sock_enotdir.sun_family = AF_UNIX;
	strncpy(sock_enotdir.sun_path, DIR_ENOTDIR "/" TEST_ENOTDIR,
		sizeof(sock_enotdir.sun_path));

	inet_socket = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	dev_null = SAFE_OPEN("/dev/null", O_WRONLY);
	fd_enotdir = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);
}

static void test_cleanup(void)
{
	if (inet_socket > 0)
		SAFE_CLOSE(inet_socket);

	if (dev_null > 0)
		SAFE_CLOSE(dev_null);

	if (fd_enotdir > 0)
		SAFE_CLOSE(fd_enotdir);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = test_setup,
	.cleanup = test_cleanup,
	.test = verify_bind,
	.needs_tmpdir = 1,
};
