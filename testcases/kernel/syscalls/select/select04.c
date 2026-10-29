// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Test to check if fd set bits are cleared by :manpage:`select(2)`.
 *
 * [Algorithm]
 *  - Check that writefds flag is cleared on full pipe
 *  - Check that readfds flag is cleared on empty pipe
 */

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "select_var.h"

static fd_set readfds_pipe, writefds_pipe;
static int fd_empty[2], fd_full[2];

static struct tcases {
	int *fd;
	fd_set *readfds;
	fd_set *writefds;
	char *desc;
} tests[] = {
	{&fd_empty[0], &readfds_pipe, NULL, "No data to read"},
	{&fd_full[1], NULL, &writefds_pipe, "No space to write"},
};

static void run(unsigned int n)
{
	struct tcases *tc = &tests[n];
	struct timeval timeout = {.tv_sec = 0, .tv_usec = 1000};

	FD_SET(fd_empty[0], &readfds_pipe);
	FD_SET(fd_full[1], &writefds_pipe);

	TEST(do_select(*tc->fd + 1, tc->readfds, tc->writefds, NULL, &timeout));

	if (TST_RET) {
		tst_res(TFAIL, "%s: select() should have timed out", tc->desc);
		return;
	}

	if ((tc->readfds && FD_ISSET(*tc->fd, tc->readfds)) ||
	    (tc->writefds && FD_ISSET(*tc->fd, tc->writefds))) {
		tst_res(TFAIL, "%s: select() didn't clear the fd set", tc->desc);
		return;
	}

	tst_res(TPASS, "%s: select() cleared the fd set", tc->desc);
}

static void setup(void)
{
	int buf = 0;

	select_info();

	SAFE_PIPE(fd_empty);
	FD_ZERO(&readfds_pipe);

	SAFE_PIPE2(fd_full, O_NONBLOCK);
	FD_ZERO(&writefds_pipe);

	/* Make the write buffer full for fd_full */
	do {
		TEST(write(fd_full[1], &buf, sizeof(buf)));
	} while (TST_RET != -1);

	if (TST_ERR != EAGAIN)
		tst_res(TFAIL | TTERRNO, "write() failed with unexpected error");
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = TEST_VARIANTS,
	.setup = setup,
	.needs_tmpdir = 1,
};
