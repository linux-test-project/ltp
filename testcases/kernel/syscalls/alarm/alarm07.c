// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Author: Wayne Boyer
 * Copyright (c) Linux Test Project, 2002-2022
 */

/*\
 * [Description]
 *
 * Verify that SIGALRM signal scheduled by alarm() in the parent process
 * is not delivered to the child process.
 */

#include <stdlib.h>
#include "tst_test.h"

static volatile int alarm_cnt;

static void verify_alarm(void)
{
	pid_t pid;

	alarm_cnt = 0;

	TST_EXP_PASS_SILENT(alarm(1));
	pid = SAFE_FORK();

	sleep(3);

	if (pid == 0) {
		TST_EXP_EQ_LU(alarm_cnt, 0);
		exit(0);
	}

	TST_EXP_EQ_LU(alarm_cnt, 1);
}

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	alarm_cnt++;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static struct tst_test test = {
	.test_all = verify_alarm,
	.setup = setup,
	.forks_child = 1,
};
