// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Author: Wayne Boyer
 *
 * Test Description:
 *  By the SIGALRM signal, check whether the previously specified alarm request
 *  was cleared in the child process or not.
 */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include "tst_test.h"

static volatile int alarm_cnt = 0;

static void verify_alarm(void)
{
	pid_t pid;
	alarm_cnt = 0;

	TEST(alarm(1));
	pid = SAFE_FORK();

	sleep(3);

	if (pid == 0) {
		if (alarm_cnt == 0) {
			tst_res(TPASS, "alarm() request cleared in child");
		} else {
			tst_res(TFAIL, "alarm() request not cleared in "
				"child; alarms received:%d", alarm_cnt);
		}
		exit(0);
	}

	if (alarm_cnt != 1)
		tst_res(TFAIL, "Sigalarms in parent %i, expected 1", alarm_cnt);
	else
		tst_res(TPASS, "Got 1 sigalarm in parent");
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
