// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Verify that getpid() system call returns process ID in range 2 ... PID_MAX
 */

#include <stdlib.h>
#include "tst_test.h"

static pid_t pid_max;

static void setup(void)
{
	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &pid_max);
}

static void verify_getpid(void)
{
	pid_t pid;
	int i;

	for (i = 0; i < 100; i++) {
		pid = SAFE_FORK();
		if (pid == 0) {
			pid = getpid();

			/* pid should not be 1 or out of maximum */
			if (1 < pid && pid <= pid_max)
				tst_res(TPASS, "getpid() returns %d", pid);
			else
				tst_res(TFAIL,
					"getpid() returns out of range: %d", pid);
			exit(0);
		} else {
			SAFE_WAIT(NULL);
		}
	}
}

static struct tst_test test = {
	.setup = setup,
	.forks_child = 1,
	.test_all = verify_getpid,
};
