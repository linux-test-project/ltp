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
	.needs_cmds = (const char *[]) {
		"mkfs.ext4",
		"mkfs.ext4 >= 1.0.0",
		"mkfs.ext4 <= 2.0.0",
		"mkfs.ext4 != 2.0.0",
		"mkfs.ext4 > 1.0.0",
		"mkfs.ext4 < 2.0.0",
		NULL
	}
};
