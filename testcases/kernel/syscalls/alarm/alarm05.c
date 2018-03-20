/*
 * Copyright (c) International Business Machines  Corp., 2001,2005
 *	07/2001 Ported by Wayne Boyer
 *	06/2005 Test for alarm cleanup by Amos Waterland
 *
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
