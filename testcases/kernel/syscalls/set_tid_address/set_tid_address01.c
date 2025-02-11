// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Linux Test Project, 2007-2024
 */

/*\
 * Verify the basic functionality of set_tid_address() syscall.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_set_tid_address(void)
{
	int newtid = -1;

	TST_EXP_VAL(tst_syscall(__NR_set_tid_address, &newtid), getpid());
}

static struct tst_test test = {
	.test_all = verify_set_tid_address,
	.needs_tmpdir = 1,
};
