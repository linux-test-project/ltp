// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*
 * Test Illegal format with garbage.
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TFAIL, "Garbage format was parsed!");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mkfs.ext4 > 1.43.0 2"},
		{}
	}
};
