// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001,2005
 * Ported to LTP: Wayne Boyer
 *	06/2005 Test for alarm cleanup by Amos Waterland
 *
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2006-2022
 */

/*\
 * [Description]
 *
 *  The return value of the alarm system call should be equal to the
 *  amount previously remaining in the alarm clock.
 *  A SIGALRM signal should be received after the specified amount of
 *  time has elapsed.
 */

#include "tst_test.h"

static volatile int alarms_fired;

static void run(void)
{
	alarms_fired = 0;

	TST_EXP_PASS(alarm(10));
	sleep(1);
	TST_EXP_VAL(alarm(1), 9);
	sleep(2);
	TST_EXP_EQ_LU(alarms_fired, 1);
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
	.timeout = 2,
	.test_all = run,
	.setup = setup,
};
