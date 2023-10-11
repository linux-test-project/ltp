// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2003-2024
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

#define SWAP_FILE "swapfile01"

static void verify_swapon(void)
{
	TST_EXP_PASS(tst_syscall(__NR_swapon, SWAP_FILE, 0));

	if (TST_PASS && tst_syscall(__NR_swapoff, SWAP_FILE) != 0) {
		tst_brk(TBROK | TERRNO,
				"Failed to turn off swapfile, system reboot recommended");
	}
}

static void setup(void)
{
	is_swap_supported(SWAP_FILE);
	make_swapfile(SWAP_FILE, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = verify_swapon,
	.setup = setup
};
