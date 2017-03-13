/*
 * Copyright (c) International Business Machines  Corp., 2005
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * AUTHORS:
 *    Prashant P Yendigeri <prashant.yendigeri@wipro.com>
 *    Robbie Williamson <robbiew@us.ibm.com>
 *
 * DESCRIPTION
 *      This is a Phase I test for the pselect01(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 **********************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_timer.h"

struct tcase {
	struct timespec tv;
	unsigned int iterations;
};

static struct tcase tcases[] = {
	{{1, 0},         1},
	{{0, 1000000}, 100},
	{{0, 2000000}, 100},
	{{0, 10000000}, 10},
	{{0, 100000000}, 1},
};

static void verify_pselect(unsigned int n)
{
	fd_set readfds;
	struct timespec tv;
	long long requested_us, slept_us = 0;
	unsigned int i;
	int threshold;
	struct tcase *t = &tcases[n];

	tst_res(TINFO, "pselect() sleeping for %li secs %li nsec %i iterations",
			t->tv.tv_sec, t->tv.tv_nsec, t->iterations);

	for (i = 0; i < t->iterations; i++) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv = t->tv;

		tst_timer_start(CLOCK_MONOTONIC);
		pselect(0, &readfds, NULL, NULL, &tv, NULL);
		tst_timer_stop();

		slept_us += tst_timer_elapsed_us();
	}

	requested_us = tst_timespec_to_us(t->tv) * t->iterations;
	threshold = requested_us / 100 + 200 * t->iterations;

	if (slept_us < requested_us) {
		tst_res(TFAIL,
			"pselect() woken up too early %llius, expected %llius",
			slept_us, requested_us);
		return;
	}

	if (slept_us - requested_us > threshold) {
		tst_res(TFAIL,
			"pselect() slept for too long %llius, expected %llius, threshold %i",
			slept_us, requested_us, threshold);
		return;
	}

	tst_res(TPASS, "pselect() slept for %llius, requested %llius, treshold %i",
		slept_us, requested_us, threshold);
}

static struct tst_test test = {
	.tid = "pselect01",
	.test = verify_pselect,
	.tcnt = ARRAY_SIZE(tcases),
};
