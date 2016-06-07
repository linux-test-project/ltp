/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
	.tid = "test13",
	.timeout = 1,
	.forks_child = 1,
	.test_all = do_test,
};
