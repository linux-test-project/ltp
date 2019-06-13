// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001,2005
 * Ported to LTP: Wayne Boyer
 *	06/2005 Test for alarm cleanup by Amos Waterland
 *
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test Description:
 *  The return value of the alarm system call should be equal to the
 *  amount previously remaining in the alarm clock.
 *  A SIGALRM signal should be received after the specified amount of
 *  time has elapsed.
 */

#include "tst_test.h"

static volatile int alarms_fired = 0;

static void run(void)
{
	unsigned int ret;

	alarms_fired = 0;

	ret = alarm(10);
	if (ret)
		tst_res(TFAIL, "alarm() returned non-zero");
	else
		tst_res(TPASS, "alarm() returned zero");

	sleep(1);

	ret = alarm(1);
	if (ret == 9)
		tst_res(TPASS, "alarm() returned remainder correctly");
	else
		tst_res(TFAIL, "alarm() returned wrong remained %u", ret);

	sleep(2);

	if (alarms_fired == 1)
		tst_res(TPASS, "alarm handler fired once");
	else
		tst_res(TFAIL, "alarm handler filred %u times", alarms_fired);
}

static void sighandler(int sig)
{
	if (sig == SIGALRM)
		alarms_fired++;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
};
