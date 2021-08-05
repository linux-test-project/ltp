// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Richard Palethorpe <rpalethorpe@suse.com>
 */
/*\
 * [Description]
 *
 * This verifies Fuzzy Sync's ability to reproduce a particular
 * outcome to a data race when multiple races are present.
 *
 * We make the simplifying assumptions that:
 * - There is one data race we want to hit and one to avoid.
 * - Each thread contains two contiguous critical sections. One for each race.
 * - The threads only interact through two variables, one for each race.
 * - If we hit the race we want to avoid then it causes thread A to exit early.
 *
 * We don't consider more complicated dynamic interactions between the
 * two threads. Fuzzy Sync will eventually trigger a race so long as
 * the delay range is large enough. Assuming the race is possible to
 * reproduce without further tampering to increase the race window (a
 * technique specific to each race). So I conject that beyond a lower
 * threshold of complexity, increasing the complexity of the race is
 * no different from adding random noise.
 *
 * Emperically this appears to be true. So far we have seen in
 * reproducers that there are no more than two significant data
 * races. One we wish to reproduce and one we wish to avoid. It is
 * possible that the code contains multiple data races, but that they
 * appear only as two to us.
 *
 * Indeed it is also only possible to add a delay to A or B. So
 * regardless of the underlying complexity we really only have two
 * options.
 *
 * Here we only test a bias to delay B. A delay of A would be
 * identical except that the necessary delay bias would be negative.
 *
\*/

#include "tst_test.h"
#include "tst_fuzzy_sync.h"

/* The time signature of a code path containing a critical section. */
struct window {
	/* The delay until the start of the critical section */
	const int critical_s;
	/* The length of the critical section */
	const int critical_t;
	/* The remaining delay until the method returns */
	const int return_t;
};

/* The time signatures of threads A and B. We interlace the two
 * windows for each thread. bd.return_t is ignored, but ad.return_t is
 * used instead of a.return_t if the ad and bd critical sections
 * overlap. This may result in the critical section of a never being
 * reached.
 */
struct race {
	const struct window ad;
	const struct window a;
	const struct window bd;
	const struct window b;
};

static int c, d;
static struct tst_fzsync_pair pair;

static const struct race races[] = {
	{ .a =  { 1, 1, 1 }, .b =  { 1, 1, 1 },
	  .ad = { 0, 1, 0 }, .bd = { 0, 1, 0 } },
	{ .a =  { 30, 1, 1 }, .b =  { 1, 1,  1 },
	  .ad = { 0,  1, 0 }, .bd = { 0, 20, 0 } },
	{ .a =  { 40, 1,  0 }, .b =  { 1, 1,  20 },
	  .ad = { 1,  10, 0 }, .bd = { 1, 10, 0 } },
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

static struct window to_abs(const struct window w)
{
	const struct window wc = {
		w.critical_s,
		w.critical_s + w.critical_t,
		w.critical_s + w.critical_t + w.return_t,
	};

	return wc;
}

static void *worker(void *v)
{
	unsigned int i = *(unsigned int *)v;
	const struct window b = to_abs(races[i].b);
	const struct window bd = to_abs(races[i].bd);
	int now, fin = MAX(b.return_t, bd.return_t);

	while (tst_fzsync_run_b(&pair)) {
		tst_fzsync_start_race_b(&pair);
		for (now = 0; now <= fin; now++) {
			if (now == b.critical_s || now == b.critical_t)
				tst_atomic_add_return(1, &c);
			if (now == bd.critical_s || now == bd.critical_t)
				tst_atomic_add_return(1, &d);

			sched_yield();
		}
		tst_fzsync_end_race_b(&pair);
	}

	return NULL;
}

static void run(unsigned int i)
{
	const struct window a = to_abs(races[i].a);
	const struct window ad = to_abs(races[i].ad);
	struct tst_fzsync_run_thread wrap_run_b = {
		.func = worker,
		.arg = &i,
	};
	int critical = 0;
	int now, fin;

	tst_fzsync_pair_reset(&pair, NULL);
	SAFE_PTHREAD_CREATE(&pair.thread_b, 0, tst_fzsync_thread_wrapper,
			    &wrap_run_b);

	while (tst_fzsync_run_a(&pair)) {
		c = 0;
		d = 0;
		fin = a.return_t;

		tst_fzsync_start_race_a(&pair);
		for (now = 0; now <= fin; now++) {
			if (now >= ad.critical_s &&
			    now <= ad.critical_t && tst_atomic_load(&d) > 0)
				fin = ad.return_t;

			if (now >= a.critical_s &&
			    now <= a.critical_t && tst_atomic_load(&c) == 1) {
				tst_atomic_add_return(1, &c);
				critical++;
			}

			sched_yield();
		}
		tst_fzsync_end_race_a(&pair);

		if (fin == ad.return_t)
			tst_fzsync_pair_add_bias(&pair, 1);

		if (critical > 100) {
			tst_fzsync_pair_cleanup(&pair);
			break;
		}
	}

	tst_res(critical > 50 ? TPASS : TFAIL, "%d| =:%-4d", i, critical);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(races),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
};
