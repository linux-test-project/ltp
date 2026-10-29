// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) Ulrich Drepper <drepper@redhat.com>
* Copyright (c) International Business Machines Corp., 2009
* Copyright (c) Linux Test Project, 2010-2022
*/

/*\
 * Test socket() with SOCK_CLOEXEC and SOCK_NONBLOCK flags.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include "lapi/fcntl.h"
#include "tst_test.h"

static int fds[2];

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

static void verify_socketpair(unsigned int n)
{
	int res, i;
	struct tcase *tc = &tcases[n];

	TEST(socketpair(PF_UNIX, tc->type, 0, fds));

	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "socketpair() failed");

	for (i = 0; i < 2; i++) {
		res = SAFE_FCNTL(fds[i], tc->fl_flag);

		if (tc->flag != 0 && (res & tc->flag) == 0) {
			tst_res(TFAIL, "socketpair() failed to set %s flag for fds[%d]",
				tc->des, i);
			goto ret;
		}

		if (tc->flag == 0 && (res & tc->flag) != 0) {
			tst_res(TFAIL, "socketpair() failed to set %s flag for fds[%d]",
				tc->des, i);
			goto ret;
		}
	}

	tst_res(TPASS, "socketpair() passed to set %s flag", tc->des);

ret:
	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);

	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_socketpair,
	.cleanup = cleanup
};
