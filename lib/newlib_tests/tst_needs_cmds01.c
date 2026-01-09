// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TPASS, "Testing tst_check_cmd() functionality OK.");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mkfs.ext4", .optional = 1},
		{.cmd = "mkfs.ext4 >= 1.0.0", .optional = 0},
		{.cmd = "mkfs.ext4 <= 2.0.0"},
		{.cmd = "mkfs.ext4 != 2.0.0"},
		{.cmd = "mkfs.ext4 > 1.0.0"},
		{.cmd = "mkfs.ext4 < 2.0.0"},
		{}
	}
};
