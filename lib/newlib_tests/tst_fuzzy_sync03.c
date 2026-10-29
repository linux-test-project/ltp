// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
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
#define LOOPS 0xFFFFULL

static volatile char seq[LOOPS * 2 + 1];
static struct tst_fzsync_pair pair;
static volatile int seq_n;
static volatile char last_wins;

static void setup(void)
{
	pair.exec_loops = LOOPS;
	tst_fzsync_pair_init(&pair);
}

static void *worker(void *v LTP_ATTRIBUTE_UNUSED)
{
	unsigned long long i;

	for (i = 0; tst_fzsync_run_b(&pair); i++) {
		tst_fzsync_start_race_b(&pair);
		usleep(1);
		last_wins = 'B';
		tst_fzsync_end_race_b(&pair);
		seq[seq_n] = 'B';
		seq_n = (i + 1) * 2 % (int)LOOPS * 2;
	}

	if (i != LOOPS) {
		tst_res(TFAIL,
			"Worker performed wrong number of iterations: %lld != %lld",
			i, LOOPS);
	}

	return NULL;
}

static void run(void)
{
	unsigned int i, j, fail = 0, lost_race = 0;

	tst_fzsync_pair_reset(&pair, worker);
	for (i = 0; tst_fzsync_run_a(&pair); i++) {
		tst_fzsync_start_race_a(&pair);
		seq[seq_n] = 'A';
		seq_n = i * 2 + 1;
		last_wins = 'A';
		tst_fzsync_end_race_a(&pair);
		if (last_wins == 'B')
			lost_race++;
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

	if (lost_race < 100)
		tst_res(TFAIL, "A only lost the race %d times", lost_race);
	else
		tst_res(TPASS, "A lost the race %d times", lost_race);
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&pair);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.runtime = 150,
};
