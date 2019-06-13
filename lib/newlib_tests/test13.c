// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

/*
 * Test for timeout & children.
 */

#include "tst_test.h"

static void do_test(void)
{
	SAFE_FORK();
	SAFE_FORK();
	SAFE_FORK();

	tst_res(TINFO, "Pausing process pid %i", getpid());
	pause();
}

static struct tst_test test = {
	.timeout = 1,
	.forks_child = 1,
	.test_all = do_test,
};
