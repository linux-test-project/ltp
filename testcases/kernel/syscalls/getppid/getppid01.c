// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2006-2023
 */

/*\
 * [Description]
 *
 * Test whether parent process id that getppid() returns is out of range.
 */

#include <errno.h>
#include "tst_test.h"

static pid_t pid_max;

static void setup(void)
{
	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &pid_max);
}

static void verify_getppid(void)
{
	pid_t ppid;

	ppid = getppid();
	if (ppid > pid_max)
		tst_res(TFAIL, "getppid() returned %d, out of range!", ppid);
	else
		tst_res(TPASS, "getppid() returned %d", ppid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_getppid,
};
