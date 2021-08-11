// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * This test is run for option 1 for sysfs(2).
 * Translate the filesystem identifier string fsname into a filesystem type index.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_sysfs01(void)
{
	/* option 1, buf holds fs name */
	TST_EXP_POSITIVE(tst_syscall(__NR_sysfs, 1, "proc"), "sysfs(1, 'proc')");
}

static struct tst_test test = {
	.test_all = verify_sysfs01,
};
