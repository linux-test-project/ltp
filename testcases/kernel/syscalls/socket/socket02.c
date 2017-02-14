/*
* Copyright (c) Ulrich Drepper <drepper@redhat.com>
* Copyright (c) International Business Machines  Corp., 2009
*
* This program is free software;  you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY;  without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
* the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/*
* Test Name:	socket02
*
* Description:
* This program tests the new flag SOCK_CLOEXEC and SOCK_NONBLOCK introduced
* in socket() in kernel 2.6.27.
*/

#include <fcntl.h>
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
	.tid = "socket02",
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_socket,
	.min_kver = "2.6.27",
	.cleanup = cleanup
};
