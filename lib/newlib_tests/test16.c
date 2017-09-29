/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* Basic functionality test for tst_fuzzy_sync.h similar to the atomic tests
 * (test15.c). One thread writes to the odd indexes of an array while the
 * other writes to the even. If the threads are not synchronised then they
 * will probably write to the wrong indexes as they share an index variable
 * which they should take it in turns to update.
 */

#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "tst_fuzzy_sync.h"

/* LOOPS * 2 + 1 must be less than INT_MAX */
#define LOOPS 0xFFFFFFULL

static pthread_t thrd;
static volatile char seq[LOOPS * 2 + 1];
static struct tst_fzsync_pair pair = TST_FZSYNC_PAIR_INIT;
static volatile int seq_n;
static volatile int iterations;

static void *worker(void *v LTP_ATTRIBUTE_UNUSED)
{
	unsigned long long i;

	for (i = 0; tst_fzsync_wait_update_b(&pair); i++) {
		tst_fzsync_delay_b(&pair);
		tst_fzsync_time_b(&pair);
		if (!tst_fzsync_wait_b(&pair))
			break;
		seq[seq_n] = 'B';
		seq_n = (i + 1) * 2 % (int)LOOPS * 2;
	}

	if (i > LOOPS * iterations)
		tst_res(TWARN, "Worker performed too many iterations: %lld > %lld",
			i, LOOPS * iterations);

	return NULL;
}

static void setup(void)
{
	SAFE_PTHREAD_CREATE(&thrd, NULL, worker, NULL);
}

static void run(void)
{
	unsigned int i, j, fail = 0;

	for (i = 0; i < LOOPS; i++) {
		tst_fzsync_wait_update_a(&pair);
		tst_fzsync_delay_a(&pair);
		seq[seq_n] = 'A';
		seq_n = i * 2 + 1;
		tst_fzsync_time_a(&pair);
		if (!tst_fzsync_wait_a(&pair))
			break;
	}

	tst_res(TINFO, "Checking sequence...");
	for (i = 0; i < LOOPS; i++) {
		j = i * 2;
		if (seq[j] != 'A') {
			tst_res(TFAIL, "Expected A, but found %c at %d",
				seq[j], j);
			fail = 1;
		}
		j = i * 2 + 1;
		if (seq[j] != 'B') {
			tst_res(TFAIL, "Expected A, but found %c at %d",
				seq[j], j);
			fail = 1;
		}
	}

	if (!fail)
		tst_res(TPASS, "Sequence is correct");

	if (labs(pair.delay) > 1000)
		tst_res(TFAIL, "Delay is suspiciously large");

	iterations++;
}

static void cleanup(void)
{
	tst_fzsync_pair_exit(&pair);
	SAFE_PTHREAD_JOIN(thrd, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
