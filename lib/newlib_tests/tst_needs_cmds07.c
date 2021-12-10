// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*
 * Test non-existed cmd whether still can be detected.
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TFAIL, "Nonexisting command is present!");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_cmds = (const char *[]) {
		"mkfs.ext45",
		NULL
	}
};
