// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Ported to LTP: Wayne Boyer
 */

/*
 * Check the functionality of the Alarm system call when the time input
 * parameter is zero.
 *
 * Expected Result:
 * The previously specified alarm request should be cancelled and the
 * SIGALRM should not be received.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "tst_test.h"

static volatile int alarms_received = 0;

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
	int ret;

	alarm(2);
	sleep(1);

	ret = alarm(0);

	/* Wait for signal SIGALRM */
	sleep(2);

	if (alarms_received)
		tst_res(TFAIL, "Received %i alarms", alarms_received);
	else
		tst_res(TPASS, "Received 0 alarms");

	if (ret == 1)
		tst_res(TPASS, "alarm(0) returned 1");
	else
		tst_res(TFAIL, "alarm(0) returned %i, expected 1", ret);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_alarm,
};
