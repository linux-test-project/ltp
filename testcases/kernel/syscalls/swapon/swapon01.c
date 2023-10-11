// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Checks that swapon() succeds with swapfile.
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

static void verify_swapon(void)
{
	TEST(tst_syscall(__NR_swapon, "./swapfile01", 0));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "Failed to turn on swapfile");
	} else {
		tst_res(TPASS, "Succeeded to turn on swapfile");
		/*we need to turn this swap file off for -i option */
		if (tst_syscall(__NR_swapoff, "./swapfile01") != 0) {
			tst_brk(TBROK | TERRNO, "Failed to turn off swapfile,"
				" system reboot after execution of LTP "
				"test suite is recommended.");
		}
	}
}

static void setup(void)
{
	is_swap_supported("./tstswap");
	make_swapfile("swapfile01", 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = verify_swapon,
	.setup = setup
};
