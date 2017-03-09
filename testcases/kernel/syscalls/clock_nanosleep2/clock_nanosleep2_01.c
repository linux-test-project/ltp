/******************************************************************************
 * Copyright (c) M. Koehrer <mathias_koehrer@arcor.de>, 2009                  *
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>                          *
 *                                                                            *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  *
 * the GNU General Public License for more details.                           *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *                                                                            *
 ******************************************************************************/
/*
 * This tests the clock_nanosleep2() syscall.
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "linux_syscall_numbers.h"

#define NSEC_IN_SEC 1000000000

const clockid_t CLOCK_TO_USE = CLOCK_MONOTONIC;
static int clock_nanosleep2(clockid_t clock_id, int flags,
			    const struct timespec *req, struct timespec *rem)
{
	return tst_syscall(__NR_clock_nanosleep, clock_id, flags, req, rem);
}

static void verify_clock_nanosleep2(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_TO_USE, &ts);
	ts.tv_nsec += NSEC_IN_SEC/10;
	if (ts.tv_nsec >= NSEC_IN_SEC) {
		ts.tv_sec += 1;
		ts.tv_nsec %= NSEC_IN_SEC;
	}

	TEST(clock_nanosleep2(CLOCK_TO_USE, TIMER_ABSTIME, &ts, NULL));

	if (TEST_RETURN)
		tst_res(TFAIL | TTERRNO, "clock_nanosleep2() failed");
	else
		tst_res(TPASS, "clock_nanosleep2() passed");
}

static struct tst_test test = {
	.tid = "clock_nanosleep2_01",
	.test_all = verify_clock_nanosleep2,
};
