// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Test Description:
 *  Verify that the system call stime() successfully sets the system's idea
 *  of date and time if invoked by "root" user.
 *
 * Expected Result:
 *  stime() should succeed to set the system data/time to the specified time.
 *
 * History
 *	07/2001 John George
 *		-Ported
 */

#include <time.h>
#include <sys/time.h>

#include "tst_test.h"
#include "stime_var.h"

static struct timeval real_time_tv;

static void run(void)
{
	time_t new_time;
	struct timeval pres_time_tv;

	if (gettimeofday(&real_time_tv, NULL) < 0)
		tst_brk(TBROK | TERRNO, "gettimeofday() failed");

	new_time = real_time_tv.tv_sec + 30;

	if (do_stime(&new_time) < 0) {
		tst_res(TFAIL | TERRNO, "stime(%ld) failed", new_time);
		return;
	}

	if (gettimeofday(&pres_time_tv, NULL) < 0)
		tst_brk(TBROK | TERRNO, "gettimeofday() failed");

	switch (pres_time_tv.tv_sec - new_time) {
	case 0:
	case 1:
		tst_res(TINFO, "pt.tv_sec: %ld", pres_time_tv.tv_sec);
		tst_res(TPASS, "system time was set to %ld", new_time);
	break;
	default:
		tst_res(TFAIL, "system time not set to %ld (got: %ld)",
			new_time, pres_time_tv.tv_sec);
	}
}

static void setup(void)
{
	stime_info();
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.restore_wallclock = 1,
	.setup = setup,
	.test_variants = TEST_VARIANTS,
};
