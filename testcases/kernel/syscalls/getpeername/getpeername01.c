/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 *  Verify that getpeername() returns the proper errno for various failure cases
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include "test.h"
#include "safe_macros.h"

static struct sockaddr_in server_addr;
static struct sockaddr_in fsin1;
static socklen_t sinlen;
static socklen_t invalid_sinlen = -1;
static int sv[2];

static void setup(void);
static void setup2(int);
static void setup3(int);
static void setup4(int);
static void cleanup(void);
static void cleanup2(int);
static void cleanup4(int);

struct test_case_t {
	int sockfd;
	struct sockaddr *sockaddr;
	socklen_t *addrlen;
	int expretval;
	int experrno;
	void (*setup) (int);
	void (*cleanup) (int);
	char *name;
} test_cases[] = {
	{-1, (struct sockaddr *)&fsin1, &sinlen, -1, EBADF, NULL, NULL,
	 "EBADF"},
	{-1, (struct sockaddr *)&fsin1, &sinlen, -1, ENOTSOCK, setup2, cleanup2,
	 "ENOTSOCK"},
	{-1, (struct sockaddr *)&fsin1, &sinlen, -1, ENOTCONN, setup3, cleanup2,
	 "ENOTCONN"},
	{-1, (struct sockaddr *)&fsin1, &invalid_sinlen, -1, EINVAL, setup4,
	 cleanup4, "EINVAL"},
#ifndef UCLINUX
	{-1, (struct sockaddr *)-1, &sinlen, -1, EFAULT, setup4, cleanup4,
	 "EFAULT"},
	{-1, (struct sockaddr *)&fsin1, NULL, -1, EFAULT, setup4,
	 cleanup4, "EFAULT"},
	{-1, (struct sockaddr *)&fsin1, (socklen_t *)1, -1, EFAULT, setup4,
	 cleanup4, "EFAULT"},
#endif
};

char *TCID = "getpeername01";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			if (test_cases[i].setup != NULL)
				test_cases[i].setup(i);

			TEST(getpeername(test_cases[i].sockfd,
					 test_cases[i].sockaddr,
					 test_cases[i].addrlen));

			if (TEST_RETURN == test_cases[i].expretval &&
			    TEST_ERRNO == test_cases[i].experrno) {
				tst_resm(TPASS,
					 "test getpeername() %s successful",
					 test_cases[i].name);
			} else {
				tst_resm(TFAIL,
					 "test getpeername() %s failed; "
					 "returned %ld (expected %d), errno %d "
					 "(expected %d)", test_cases[i].name,
					 TEST_RETURN, test_cases[i].expretval,
					 TEST_ERRNO, test_cases[i].experrno);
			}

			if (test_cases[i].cleanup != NULL)
				test_cases[i].cleanup(i);
		}
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = 0;
	server_addr.sin_addr.s_addr = INADDR_ANY;

	sinlen = sizeof(fsin1);
}

static void cleanup(void)
{
}

static void setup2(int i)
{
	test_cases[i].sockfd = SAFE_OPEN(cleanup, "/dev/null", O_WRONLY, 0666);
}

static void setup3(int i)
{
	test_cases[i].sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (test_cases[i].sockfd < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "socket setup failed for getpeername test %d", i);
	}
	SAFE_BIND(cleanup, test_cases[i].sockfd,
		  (struct sockaddr *)&server_addr, sizeof(server_addr));
}

static void setup4(int i)
{
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "socketpair failed for getpeername test %d", i);
	}
	test_cases[i].sockfd = sv[0];
}

static void cleanup2(int i)
{
	SAFE_CLOSE(cleanup, test_cases[i].sockfd);
}

static void cleanup4(int i)
{
	SAFE_CLOSE(cleanup, sv[0]);
	SAFE_CLOSE(cleanup, sv[1]);
}
