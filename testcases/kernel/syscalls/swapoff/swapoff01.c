// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Check that swapoff() succeeds.
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "libswap.h"

static void verify_swapoff(void)
{
	if (tst_syscall(__NR_swapon, "./swapfile01", 0) != 0) {
		tst_res(TFAIL | TERRNO, "Failed to turn on the swap file"
			 ", skipping test iteration");
		return;
	}

	TEST(tst_syscall(__NR_swapoff, "./swapfile01"));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "Failed to turn off swapfile,"
			" system reboot after execution of LTP "
			"test suite is recommended.");
	} else {
		tst_res(TPASS, "Succeeded to turn off swapfile");
	}
}

static void setup(void)
{
	is_swap_supported("./tstswap");

	if (!tst_fs_has_free(".", 64, TST_MB))
		tst_brk(TBROK,
			"Insufficient disk space to create swap file");

	if (make_swapfile("swapfile01", 65536, 1))
		tst_brk(TBROK, "Failed to create file for swap");
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = verify_swapoff,
	.setup = setup
};
