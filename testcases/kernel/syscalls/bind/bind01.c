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

int inet_socket;
int dev_null;

struct sockaddr_in sin1, sin2, sin3;
struct sockaddr_un sun;

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
};

void verify_bind(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	TEST(bind(*tcase->socket_fd, tcase->sockaddr, tcase->salen));
	if (TST_RET != tcase->retval && TST_ERR != tcase->experrno) {
		tst_res(TFAIL, "%s ; returned"
			" %ld (expected %d), errno %d (expected"
			" %d)", tcase->desc, TST_RET, tcase->retval,
			TST_ERR, tcase->experrno);
	} else {
		tst_res(TPASS, "%s successful", tcase->desc);
	}
}

void test_setup(void)
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

	inet_socket = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);
	dev_null = SAFE_OPEN("/dev/null", O_WRONLY);
}

void test_cleanup(void)
{
	SAFE_CLOSE(inet_socket);
	SAFE_CLOSE(dev_null);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = test_setup,
	.cleanup = test_cleanup,
	.test = verify_bind,
};
