// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Richard Logan
 * Copyright (c) Linux Test Project, 2001-2022
 */

/*\
 * [Description]
 *
 * Verify that alarms created by alarm() are not inherited by children
 * created via fork.
 */

#include <stdlib.h>
#include "tst_test.h"

static void verify_alarm(void)
{
	pid_t pid;

	TST_EXP_PASS_SILENT(alarm(100));

	pid = SAFE_FORK();
	if (pid == 0) {
		TST_EXP_PASS(alarm(0), "alarm(0) in child process");
		exit(0);
	}

	TST_EXP_VAL(alarm(0), 100, "alarm(0) in parent process");
}

static struct tst_test test = {
	.test_all = verify_alarm,
	.forks_child = 1,
};
