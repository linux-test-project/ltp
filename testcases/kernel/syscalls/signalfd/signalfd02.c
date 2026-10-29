// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Ma Xinjian <maxj.fnst@fujitsu.com>
 */

/*\
 * Verify that signalfd(2) fails with:
 *
 * - EBADF when fd is invalid
 * - EINVAL when fd is not a valid signalfd file descriptor
 * - EINVAL when flags are invalid
 */

#include <sys/signalfd.h>
#include "tst_test.h"

#define SIGNAL_FILE "signal_file"

static int fd_ebadf = -2;
static int fd_einval1;
static int fd_einval2 = -1;

static sigset_t *mask;

static struct test_case_t {
	int *fd;
	int flags;
	int expected_errno;
	char *desc;
} tcases[] = {
	{&fd_ebadf, 0, EBADF, "fd is invalid"},
	{&fd_einval1, 0, EINVAL,
		"fd is not a valid signalfd file descriptor"},
	{&fd_einval2, -1, EINVAL, "flags are invalid"},
};

static void setup(void)
{
	SAFE_SIGEMPTYSET(mask);
	SAFE_SIGADDSET(mask, SIGUSR1);
	SAFE_SIGPROCMASK(SIG_BLOCK, mask, NULL);

	fd_einval1 = SAFE_OPEN(SIGNAL_FILE, O_CREAT, 0777);
}

static void cleanup(void)
{
	if (fd_einval1 > 0)
		SAFE_CLOSE(fd_einval1);
}

static void verify_signalfd(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];

	TST_EXP_FAIL2(signalfd(*(tc->fd), mask, tc->flags),
		tc->expected_errno, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_signalfd,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&mask, .size = sizeof(sigset_t)},
		{}
	}
};
