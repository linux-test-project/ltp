/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Check the functionality of the Alarm system call when the time input
 *  parameter is zero.
 *
 * Expected Result:
 *  The previously specified alarm request should be cancelled and the
 *  SIGALRM should not be received.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
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
	.tid = "alarm06",
	.setup = setup,
	.test_all = verify_alarm,
};
