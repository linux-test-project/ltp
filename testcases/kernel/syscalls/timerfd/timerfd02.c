// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */
/*\
 * [Description]
 *
 * This test verifies that:
 * - TFD_CLOEXEC sets the close-on-exec file status flag on the new open  file
 * - TFD_NONBLOCK sets the O_NONBLOCK file status flag on the new open file
 */

#include "tst_test.h"
#include "tst_safe_timerfd.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

static int fdesc;

static struct test_case_t {
	int flags;
	int check;
	int expected;
} tcases[] = {
	{ 0, F_GETFD, 0 },
	{ TFD_CLOEXEC, F_GETFD, FD_CLOEXEC },
	{ TFD_NONBLOCK, F_GETFL, O_NONBLOCK }
};

static void run(unsigned int i)
{
	struct test_case_t *tcase = &tcases[i];

	TST_EXP_FD(fdesc = SAFE_TIMERFD_CREATE(CLOCK_REALTIME, tcase->flags));
	TST_EXP_EQ_LI(SAFE_FCNTL(fdesc, tcase->check) & tcase->expected, tcase->expected);
	SAFE_CLOSE(fdesc);
}

static void cleanup(void)
{
	if (fcntl(fdesc, F_GETFD) != -1)
		SAFE_CLOSE(fdesc);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
};
