// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Test whether parent process id that getppid() returns is out of range.
 */

#include <errno.h>
#include "tst_test.h"

static void verify_getppid(void)
{
	pid_t ppid, pid_max;

	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &pid_max);

	ppid = getppid();
	if (ppid > pid_max)
		tst_res(TFAIL, "getppid() returned %d, out of range!", ppid);
	else
		tst_res(TPASS, "getppid() returned %d", ppid);
}

static struct tst_test test = {
	.test_all = verify_getppid,
};
