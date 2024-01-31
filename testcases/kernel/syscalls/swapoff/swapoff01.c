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

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/testswap"
#define SWAP_FILE	MNTPOINT"/swapfile"

static void verify_swapoff(void)
{
	if (tst_syscall(__NR_swapon, SWAP_FILE, 0) != 0) {
		tst_res(TFAIL | TERRNO, "Failed to turn on the swap file"
			 ", skipping test iteration");
		return;
	}

	TEST(tst_syscall(__NR_swapoff, SWAP_FILE));

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
	is_swap_supported(TEST_FILE);

	if (make_swapfile(SWAP_FILE, 65536, 1))
		tst_brk(TBROK, "Failed to create file for swap");
}

static struct tst_test test = {
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.dev_min_size = 350,
	.all_filesystems = 1,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = verify_swapoff,
	.setup = setup
};
