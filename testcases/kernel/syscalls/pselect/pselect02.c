// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 *  Verify that pselect() fails with:
 *
 *  - EBADF if a file descriptor that was already closed
 *  - EINVAL if nfds was negative
 *  - EINVAL if the value contained within timeout was invalid
 */

#include "tst_test.h"

static fd_set read_fds;
static struct timespec time_buf;

static struct tcase {
	int nfds;
	fd_set *readfds;
	struct timespec *timeout;
	int exp_errno;
} tcases[] = {
	{128, &read_fds, NULL, EBADF},
	{-1, NULL, NULL, EINVAL},
	{128, NULL, &time_buf, EINVAL},
};

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN("test_file", O_RDWR | O_CREAT, 0777);

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	SAFE_CLOSE(fd);

	time_buf.tv_sec = -1;
	time_buf.tv_nsec = 0;
}

static void pselect_verify(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(pselect(tc->nfds, tc->readfds, NULL, NULL, tc->timeout, NULL),
			tc->exp_errno, "pselect(%i, %p, %p)",
			tc->nfds, tc->readfds, tc->timeout);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = pselect_verify,
	.setup = setup,
	.needs_tmpdir = 1,
};
