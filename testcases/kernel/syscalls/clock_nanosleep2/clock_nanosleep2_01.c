// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) M. Koehrer <mathias_koehrer@arcor.de>, 2009
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

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

	if (TST_RET)
		tst_res(TFAIL | TTERRNO, "clock_nanosleep2() failed");
	else
		tst_res(TPASS, "clock_nanosleep2() passed");
}

static struct tst_test test = {
	.test_all = verify_clock_nanosleep2,
};
