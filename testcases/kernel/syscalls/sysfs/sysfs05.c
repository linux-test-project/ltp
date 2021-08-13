// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * This test case checks whether sysfs(2) system call returns appropriate
 * error number for invalid option and for invalid filesystem name and fs index out of bounds.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static struct test_case {
	int option;
	char *fsname;
	int fsindex;
	char *err_desc;
	int exp_errno;
} tcases[] = {
	{1, "ext0", 0, "Invalid filesystem name", EINVAL},
	{4, NULL, 0, "Invalid option", EINVAL},
	{1, NULL, 0, "Address is out of your address space", EFAULT},
	{2, NULL, 1000, "fs_index is out of bounds", EINVAL}
};

static void verify_sysfs05(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];
	char buf[1024];

	if (tc->option == 1) {
		TST_EXP_FAIL(tst_syscall(__NR_sysfs, tc->option, tc->fsname),
					tc->exp_errno, "%s", tc->err_desc);
	} else {
		TST_EXP_FAIL(tst_syscall(__NR_sysfs, tc->option, tc->fsindex, buf),
					tc->exp_errno, "%s", tc->err_desc);
	}
}

static void setup(void)
{
	unsigned int i;
	char *bad_addr;

	bad_addr = tst_get_bad_addr(NULL);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].exp_errno == EFAULT)
			tcases[i].fsname = bad_addr;
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_sysfs05,
};

