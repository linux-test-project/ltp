// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * This test case checks whether sysfs(2) system call returns
 * appropriate error number for invalid option.
 */

#include <errno.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define INVALID_OPTION 100

static void verify_sysfs04(void)
{
	TST_EXP_FAIL(tst_syscall(__NR_sysfs, INVALID_OPTION),
				EINVAL, "sysfs(INVALID_OPTION)");
}

static struct tst_test test = {
	.test_all = verify_sysfs04,
};
