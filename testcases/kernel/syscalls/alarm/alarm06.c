// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2002-2022
 * Ported to LTP: Wayne Boyer
 */

/*\
 * Verify that any pending alarm() is canceled when seconds is zero.
 */

#include "tst_test.h"

static volatile int alarms_received;

static void sigproc(int sig)
{
	if (sig == SIGALRM)
		alarms_received++;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGALRM, sigproc);
}

static void verify_alarm(void)
{
	TST_EXP_PASS_SILENT(alarm(2));
	sleep(1);

	TST_EXP_VAL(alarm(0), 1);

	/* Wait for signal SIGALRM */
	sleep(2);

	TST_EXP_EQ_LU(alarms_received, 0);
}

static struct tst_test test = {
	.timeout = 4,
	.setup = setup,
	.test_all = verify_alarm,
};
