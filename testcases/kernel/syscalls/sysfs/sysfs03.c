// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * This test is run for option 3 for sysfs(2).
 * Return the total number of filesystem types currently present in the kernel.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_sysfs03(void)
{
	TST_EXP_POSITIVE(tst_syscall(__NR_sysfs, 3), "sysfs(3)");
}

static struct tst_test test = {
	.test_all = verify_sysfs03,
};
