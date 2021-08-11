// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * This test is run for option 2 for sysfs(2).
 * Translate the filesystem type index fs_index into a null-terminated filesystem
 * identifier string. This string will be written to the buffer pointed to by buf.
 * Make sure that buf has enough space to accept the string.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_sysfs02(void)
{
	char buf[40];

	TST_EXP_PASS(tst_syscall(__NR_sysfs, 2, 0, buf), "sysfs(2,0,buf)");
}

static struct tst_test test = {
	.test_all = verify_sysfs02,
};
