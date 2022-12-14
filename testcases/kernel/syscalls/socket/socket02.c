// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) Ulrich Drepper <drepper@redhat.com>
* Copyright (c) International Business Machines Corp., 2009
*/

/*
* Test Name:	socket02
*
* Description:
* This program tests the new flag SOCK_CLOEXEC and SOCK_NONBLOCK introduced
* in socket() in kernel 2.6.27.
*/

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "lapi/fcntl.h"
#include "tst_test.h"

static int fd;

static struct tcase {
	int type;
	int flag;
	int fl_flag;
	char *des;
} tcases[] = {
	{SOCK_STREAM, 0, F_GETFD, "no close-on-exec"},
	{SOCK_STREAM | SOCK_CLOEXEC, FD_CLOEXEC, F_GETFD, "close-on-exec"},
	{SOCK_STREAM, 0, F_GETFL, "no non-blocking"},
	{SOCK_STREAM | SOCK_NONBLOCK, O_NONBLOCK, F_GETFL, "non-blocking"}
};

static void verify_socket(unsigned int n)
{
	int res;
	struct tcase *tc = &tcases[n];

	fd = socket(PF_INET, tc->type, 0);
	if (fd == -1)
		tst_brk(TFAIL | TERRNO, "socket() failed");

	res = SAFE_FCNTL(fd, tc->fl_flag);

	if (tc->flag != 0 && (res & tc->flag) == 0) {
		tst_res(TFAIL, "socket() failed to set %s flag", tc->des);
		return;
	}

	if (tc->flag == 0 && (res & tc->flag) != 0) {
		tst_res(TFAIL, "socket() failed to set %s flag", tc->des);
		return;
	}

	tst_res(TPASS, "socket() passed to set %s flag", tc->des);

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_socket,
	.cleanup = cleanup
};
