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

static unsigned int monotonic_resolution;

static struct tcase tcases[] = {
	{{0, 1000000},  500},
	{{0, 2000000},  500},
	{{0, 10000000}, 300},
	{{0, 100000000},  1},
	{{1, 0},          1},
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * The threshold per one syscall is computed as a sum of:
 *
 *  250 us                 - accomodates for context switches, etc.
 *  2*monotonic_resolution - accomodates for granurality of the CLOCK_MONOTONIC
 *  slack_per_scall        - 0.1% of the sleep capped on 100ms
 *                           which is slack allowed in kernel
 *
 * We also allow for outliners, i.e. add some number to the threshold in case
 * that the number of iteration is small. For large enoung number of iterations
 * outliners are averaged out.
 */
static int compute_threshold(long long requested_us, unsigned int iterations)
{
	unsigned int slack_per_scall = MIN(100000, requested_us / 1000);

	return (250 + 2 * monotonic_resolution + slack_per_scall) * iterations
		+ (iterations > 1 ? 0 : 1500);
}

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
		long long elapsed_us;

		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv = t->tv;

		tst_timer_start(CLOCK_MONOTONIC);
		pselect(0, &readfds, NULL, NULL, &tv, NULL);
		tst_timer_stop();

		elapsed_us = tst_timer_elapsed_us();

		if (elapsed_us >= 10 * tst_timespec_to_us(t->tv)
		    && elapsed_us > 3 * monotonic_resolution) {
			tst_res(TINFO,
				"Found outliner took %lli us, expected %lli us",
				elapsed_us, tst_timespec_to_us(t->tv));
		}

		slept_us += elapsed_us;
	}

	requested_us = tst_timespec_to_us(t->tv) * t->iterations;
	threshold = compute_threshold(tst_timespec_to_us(t->tv), t->iterations);

	if (t->iterations > 1) {
		tst_res(TINFO, "Mean sleep time %.2f us, expected %lli us, threshold %.2f",
			1.00 * slept_us / t->iterations,
			tst_timespec_to_us(t->tv), 1.00 * threshold / t->iterations);
	}

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

static void setup(void)
{
	struct timespec t;

	clock_getres(CLOCK_MONOTONIC, &t);

	tst_res(TINFO, "CLOCK_MONOTONIC resolution %li ns", (long)t.tv_nsec);

	monotonic_resolution = t.tv_nsec / 1000;
}

static struct tst_test test = {
	.tid = "pselect01",
	.test = verify_pselect,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
};
