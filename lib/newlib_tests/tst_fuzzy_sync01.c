// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Richard Palethorpe <rpalethorpe@suse.com>
 */
/*\
 * [Description]
 *
 * This verifies Fuzzy Sync's basic ability to reproduce a particular
 * outcome to a data race when the critical sections are not aligned.
 *
 * We make the simplifying assumptions that:
 * - Each thread contains a single contiguous critical section.
 * - The threads only interact through a single variable.
 * - The various timings are constant except for variations introduced
 *   by the environment.
 *
 * If a single data race has N critical sections then we may remove
 * N-1 sections to produce a more difficult race. We may then test
 * only the more difficult race and induce from this the outcome of
 * testing the easier races.
 *
 * In real code, the threads may interact through many side
 * effects. While some of these side effects may not result in a bug,
 * they may effect the total time it takes to execute either
 * thread. This will be handled in tst_fuzzy_sync02.
 *
 * The number of variables which two threads interact through is
 * irrelevant as the combined state of two variables can be
 * represented with a single variable. We may also reduce the number
 * of states to simply those required to show the thread is inside or
 * outside of the critical section.
 *
 * There are two fundamental races which require alignment under these
 * assumptions:
 *      1        2
 * A +-----+  +----+	The outer box is total execution time.
 *   | #   |  | #  |	The '#' is the critical section.
 *
 *   |  # |   |   # |
 * B +----+   +-----+
 *
 * So we can either have the critical section of the shorter race
 * before that of the longer one. Or the critical section of the
 * longer one before the shorter.
 *
 * In reality both threads will never be the same length, but we can
 * test that anyway. We also test with both A as the shorter and B as
 * the shorter. We also vary the distance of the critical section from
 * the start or end. The delay times are cubed to ensure that a delay
 * range is required.
 *
 * When entering their critical sections, both threads increment the
 * 'c' counter variable atomically. They both also increment it when
 * leaving their critical sections. We record the value of 'c' when A
 * increments it. From the recorded values of 'c' we can deduce if the
 * critical sections overlap and their ordering.
 *
 * 	Start (cs)	| End (ct)	| Ordering
 * 	--------------------------------------------
 * 	1		| 2		| A before B
 * 	3		| 4		| B before A
 *
 * Any other combination of 'cs' and 'ct' means the critical sections
 * overlapped.
\*/

#include "tst_test.h"
#include "tst_fuzzy_sync.h"

/* Scale all the delay times by this function. The races become harder
 * the faster this function grows. With cubic scaling the race windows
 * will be 27 times smaller than the entry or return delays. Because
 * TIME_SCALE(1) = 1*1*1, TIME_SCALE(3) = 3*3*3.
 */
#define TIME_SCALE(x) ((x) * (x) * (x))

/* The time signature of a code path containing a critical section. */
struct window {
	/* The delay until the start of the critical section */
	const int critical_s;
	/* The length of the critical section */
	const int critical_t;
	/* The remaining delay until the method returns */
	const int return_t;
};

/* The time signatures of threads A and B */
struct race {
	const struct window a;
	const struct window b;
};

static int c;
static struct tst_fzsync_pair pair;

static const struct race races[] = {
	/* Degnerate cases where the critical sections are already
	 * aligned. The first case will fail when ncpu < 2 as a yield
	 * inside the critical section is required for the other
	 * thread to run.
	 */
	{ .a = { 0, 0, 0 }, .b = { 0, 0, 0 } },
	{ .a = { 0, 1, 0 }, .b = { 0, 1, 0 } },
	{ .a = { 1, 1, 1 }, .b = { 1, 1, 1 } },
	{ .a = { 3, 1, 1 }, .b = { 3, 1, 1 } },

	/* Both windows are the same length */
	{ .a = { 3, 1, 1 }, .b = { 1, 1, 3 } },
	{ .a = { 1, 1, 3 }, .b = { 3, 1, 1 } },

	/* Different sized windows */
	{ .a = { 3, 1, 1 }, .b = { 1, 1, 2 } },
	{ .a = { 1, 1, 3 }, .b = { 2, 1, 1 } },
	{ .a = { 2, 1, 1 }, .b = { 1, 1, 3 } },
	{ .a = { 1, 1, 2 }, .b = { 3, 1, 1 } },

	/* Same as above, but with critical section at entry or exit */
	{ .a = { 3, 1, 0 }, .b = { 0, 1, 3 } },
	{ .a = { 0, 1, 3 }, .b = { 3, 1, 0 } },

	{ .a = { 3, 1, 0 }, .b = { 0, 1, 2 } },
	{ .a = { 0, 1, 3 }, .b = { 2, 1, 0 } },
	{ .a = { 2, 1, 0 }, .b = { 0, 1, 3 } },
	{ .a = { 0, 1, 2 }, .b = { 3, 1, 0 } },

	/* One side is very short */
	{ .a = { 3, 1, 1 }, .b = { 0, 1, 0 } },
	{ .a = { 1, 1, 3 }, .b = { 0, 1, 0 } },
	{ .a = { 0, 1, 0 }, .b = { 1, 1, 3 } },
	{ .a = { 0, 1, 0 }, .b = { 3, 1, 1 } },

	{ .a = { 3, 1, 1 }, .b = { 0, 0, 0 } },
	{ .a = { 1, 1, 3 }, .b = { 0, 0, 0 } },
	{ .a = { 0, 0, 0 }, .b = { 1, 1, 3 } },
	{ .a = { 0, 0, 0 }, .b = { 3, 1, 1 } },

};

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&pair);
}

static void setup(void)
{
	pair.min_samples = 10000;

	tst_fzsync_pair_init(&pair);
}

static void delay(const int t)
{
	int k = TIME_SCALE(t);

	while (k--)
		sched_yield();
}

static void *worker(void *v)
{
	unsigned int i = *(unsigned int *)v;
	const struct window b = races[i].b;

	while (tst_fzsync_run_b(&pair)) {
		if (tst_atomic_load(&c))
			tst_brk(TBROK, "Counter should now be zero");

		tst_fzsync_start_race_b(&pair);
		delay(b.critical_s);

		tst_atomic_add_return(1, &c);
		delay(b.critical_t);
		tst_atomic_add_return(1, &c);

		delay(b.return_t);
		tst_fzsync_end_race_b(&pair);
	}

	return NULL;
}

static void run(unsigned int i)
{
	const struct window a = races[i].a;
	int cs, ct, r, too_early = 0, critical = 0, too_late = 0;

	tst_fzsync_pair_reset(&pair, NULL);
	SAFE_PTHREAD_CREATE(&pair.thread_b, 0, worker, &i);

	while (tst_fzsync_run_a(&pair)) {

		tst_fzsync_start_race_a(&pair);
		delay(a.critical_s);

		cs = tst_atomic_add_return(1, &c);
		delay(a.critical_t);
		ct = tst_atomic_add_return(1, &c);

		delay(a.return_t);
		tst_fzsync_end_race_a(&pair);

		if (cs == 1 && ct == 2)
			too_early++;
		else if (cs == 3 && ct == 4)
			too_late++;
		else
			critical++;

		r = tst_atomic_add_return(-4, &c);
		if (r)
			tst_brk(TBROK, "cs = %d, ct = %d, r = %d", cs, ct, r);

		if (critical > 100) {
			tst_fzsync_pair_cleanup(&pair);
			break;
		}
	}

	tst_res(critical > 50 ? TPASS : TFAIL,
		"acs:%-2d act:%-2d art:%-2d | =:%-4d -:%-4d +:%-4d",
		a.critical_s, a.critical_t, a.return_t,
		critical, too_early, too_late);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(races),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.max_runtime = 150,
};
