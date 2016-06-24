/*
* Copyright (c) International Business Machines  Corp., 2001
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
* Test Name: socket01
*
* Test Description:
* Verify that socket() returns the proper errno for various failure cases
*
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "tst_test.h"

struct test_case_t {
	int domain;
	int type;
	int proto;
	int retval;
	int experrno;
	char *desc;
} tdat[] = {
	{0, SOCK_STREAM, 0, -1, EAFNOSUPPORT, "invalid domain"},
	{PF_INET, 75, 0, -1, EINVAL, "invalid type"},
	{PF_UNIX, SOCK_DGRAM, 0, 0, 0, "UNIX domain dgram"},
	{PF_INET, SOCK_RAW, 0, -1, EPROTONOSUPPORT, "raw open as non-root"},
	{PF_INET, SOCK_DGRAM, 17, 0, 0, "UDP socket"},
	{PF_INET, SOCK_STREAM, 17, -1, EPROTONOSUPPORT, "UDP stream"},
	{PF_INET, SOCK_DGRAM, 6, -1, EPROTONOSUPPORT, "TCP dgram"},
	{PF_INET, SOCK_STREAM, 6, 0, 0, "TCP socket"},
	{PF_INET, SOCK_STREAM, 1, -1, EPROTONOSUPPORT, "ICMP stream"}
};

static void verify_socket(unsigned int n)
{
	int fd;
	struct test_case_t *tc = &tdat[n];

	TEST(fd = socket(tc->domain, tc->type, tc->proto));
	if (TEST_RETURN >= 0)
		TEST_RETURN = 0;

	if (fd > 0)
		SAFE_CLOSE(fd);

	if (TEST_RETURN != tc->retval) {
		tst_res(TFAIL, "%s returned %d (expected %d)",
			tc->desc, fd, tc->retval);
		return;
	}

	if (TEST_ERRNO != tc->experrno) {
		tst_res(TFAIL | TTERRNO, "expected %s(%d)",
		        tst_strerrno(tc->experrno), tc->experrno);
		return;
	}

	tst_res(TPASS, "%s successful", tc->desc);
}

/*
 * See:
 * commit 86c8f9d158f68538a971a47206a46a22c7479bac
 * ...
 * [IPV4] Fix EPROTONOSUPPORT error in inet_create
 */
static void setup(void)
{
	unsigned int i;

	if (tst_kvercmp(2, 6, 16) >= 0)
		return;

	for (i = 0; i < ARRAY_SIZE(tdat); i++) {
		if (tdat[i].experrno == EPROTONOSUPPORT)
				tdat[i].experrno = ESOCKTNOSUPPORT;
	}
}

static struct tst_test test = {
	.tid = "socket01",
	.tcnt = ARRAY_SIZE(tdat),
	.setup = setup,
	.test = verify_socket
};
