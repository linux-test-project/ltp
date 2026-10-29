// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*
 * Test mkfs.xfs that it doesn't have own parser and table_get function
 * at the version_parsers structure in lib/tst_cmd.c.
 * So it should report parser function for this cmd is not implemented.
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TFAIL, "Nonexisting parser function for mkfs.xfs is present!");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mkfs.xfs"},
		{.cmd = "mkfs.xfs >= 4.20.0"},
		{}
	}
};
