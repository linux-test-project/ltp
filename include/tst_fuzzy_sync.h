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
/*
 * Fuzzy Synchronisation - abreviated to fzsync
 *
 * This library is intended to help reproduce race conditions by providing two
 * thread synchronisation mechanisms. The first is a 'checkpoint' system where
 * each thread will wait indefinitely for the other to enter a checkpoint
 * before continuing. This is acheived by calling tst_fzsync_wait() and/or
 * tst_fzsync_wait_update() at the points you want to synchronise in each
 * thread. This is functionally very similar to using pthread_barrier_wait()
 * with two threads. However tst_fzsync_wait() can be inlined and is
 * guaranteed not to call sleep or futex.
 *
 * The second takes the form a of a delay which is calculated by measuring the
 * time difference between two points in each thread and comparing it to the
 * desired difference (default is zero). Using a delay allows you to
 * synchronise the threads at a point outside of your direct control
 * (e.g. inside the kernel) or just increase the accuracy for the first
 * mechanism. It is acheived using tst_fzsync_delay_{a,b}(),
 * tst_fzsync_time_{a,b}() and tst_fzsync[_wait_]update().
 *
 * For a usage example see testcases/cve/cve-2016-7117.c or just run
 * 'git grep tst_fuzzy_sync.h'
 */

#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "tst_atomic.h"

#ifndef CLOCK_MONOTONIC_RAW
# define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

/**
 * struct tst_fzsync_pair - the state of a two way synchronisation
 * @avg_diff: Internal; the average time difference over multiple iterations.
 * @avg_diff_trgt: The desired average time difference, defaults to 0.
 * @avg_alpha: The rate at which old diff samples are forgotten,
 *             defaults to 0.25.
 * @avg_dev: Internal; Absolute average deviation.
 * @a: Internal; The time at which call site A was last passed.
 * @b: Internal; The time at which call site B was last passed.
 * @delay: Internal; The size of the delay, positive to delay A,
 *         negative to delay B.
 * @delay_inc: The step size of a delay increment, defaults to 1.
 * @update_gap: The number of iterations between recalculating the delay.
 *              Defaults to 0xF and must be of the form $2^n - 1$
 * @info_gap: The number of iterations between printing some statistics.
 *            Defaults to 0x7FFFF and must also be one less than a power of 2.
 * @a_cntr: Internal; Atomic counter used by fzsync_pair_wait()
 * @b_cntr: Internal; Atomic counter used by fzsync_pair_wait()
 * @exit: Internal; Used by tst_fzsync_pair_exit() and fzsync_pair_wait()
 *
 * This contains all the necessary state for synchronising two points A and
 * B. Where A is the time of an event in one process and B is the time of an
 * event in another process.
 *
 * Internal fields should only be accessed by library functions.
 */
struct tst_fzsync_pair {
	float avg_diff;
	float avg_diff_trgt;
	float avg_alpha;
	float avg_dev;
	struct timespec a;
	struct timespec b;
	long delay;
	long delay_inc;
	int update_gap;
	int info_gap;
	int a_cntr;
	int b_cntr;
	int exit;
};

/**
 * TST_FZSYNC_PAIR_INIT - Default values for struct tst_fzysnc_pair
 */
#define TST_FZSYNC_PAIR_INIT {	\
	.avg_alpha = 0.25,	\
	.delay_inc = 1,	        \
	.update_gap = 0xF,	\
	.info_gap = 0x7FFFF     \
}

/**
 * tst_fzsync_pair_info - Print some synchronisation statistics
 */
static void tst_fzsync_pair_info(struct tst_fzsync_pair *pair)
{
	tst_res(TINFO,
		"avg_diff = %.0fns, avg_dev = %.0fns, delay = %05ld loops",
		pair->avg_diff, pair->avg_dev, pair->delay);
}

/**
 * tst_fzsync_delay_a - Perform spin delay for A, if needed
 *
 * Usually called just before the point you want to synchronise.
 */
static inline void tst_fzsync_delay_a(struct tst_fzsync_pair *pair)
{
	volatile long spin_delay = pair->delay;

	while (spin_delay > 0)
		spin_delay--;
}

/**
 * tst_fzsync_delay_b - Perform spin delay for B, if needed
 *
 * Usually called just before the point you want to synchronise.
 */
static inline void tst_fzsync_delay_b(struct tst_fzsync_pair *pair)
{
	volatile long spin_delay = pair->delay;

	while (spin_delay < 0)
		spin_delay++;
}

static inline void tst_fzsync_time(struct timespec *t)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, t);
}

/**
 * tst_fzsync_time_a - Set A's time to now.
 *
 * Called at the point you want to synchronise.
 */
static inline void tst_fzsync_time_a(struct tst_fzsync_pair *pair)
{
	tst_fzsync_time(&pair->a);
}

/**
 * tst_fzsync_time_b - Set B's call time to now.
 *
 * Called at the point you want to synchronise.
 */
static inline void tst_fzsync_time_b(struct tst_fzsync_pair *pair)
{
	tst_fzsync_time(&pair->b);
}

/**
 * tst_exp_moving_avg - Exponential moving average
 * @alpha: The preference for recent samples over old ones.
 * @sample: The current sample
 * @prev_avg: The average of the all the previous samples
 *
 * Returns average including the current sample.
 */
static inline float tst_exp_moving_avg(float alpha,
					float sample,
					float prev_avg)
{
	return alpha * sample + (1.0 - alpha) * prev_avg;
}

/**
 * tst_fzsync_pair_update - Recalculate the delay
 * @loop_index: The i in "for(i = 0;..." or zero to ignore update_gap
 * @pair: The state necessary for calculating the delay
 *
 * This should be called at the end of each loop to update the average
 * measured time difference (between A and B) and update the delay. It is
 * assumed that A and B are less than a second apart.
 *
 * The values of update_gap, avg_alpha and delay_inc decide the speed at which
 * the algorithm approaches the optimum delay value and whether it is
 * stable. If your test is behaving strangely, it could be because this
 * algorithm is behaving chaotically and flip-flopping between large positve
 * and negative delay values. You can call tst_fzysync_pair_info every few
 * loops to check whether the average difference and delay values are stable.
 */
static void tst_fzsync_pair_update(int loop_index, struct tst_fzsync_pair *pair)
{
	long diff;
	long inc = pair->delay_inc;
	float target = pair->avg_diff_trgt;
	float avg = pair->avg_diff;

	diff = pair->a.tv_nsec - pair->b.tv_nsec
		+ 1000000000 * (pair->a.tv_sec - pair->b.tv_sec);
	avg = tst_exp_moving_avg(pair->avg_alpha, diff, avg);
	pair->avg_dev = tst_exp_moving_avg(pair->avg_alpha,
					   fabs(diff - avg),
					   pair->avg_dev);

	if (!(loop_index & pair->update_gap)) {
		if (avg > target)
			pair->delay -= inc;
		else if (avg < target)
			pair->delay += inc;
	}

	if (!(loop_index & pair->info_gap))
		tst_fzsync_pair_info(pair);

	pair->avg_diff = avg;
}

/**
 * tst_fzsync_pair_wait - Wait for the other thread
 * @our_cntr: The counter for the thread we are on
 * @other_cntr: The counter for the thread we are synchronising with
 *
 * Use this (through tst_fzsync_pair_wait_a() and tst_fzsync_pair_wait_b()) if
 * you need an additional synchronisation point in a thread or you do not want
 * to use the delay facility (not recommended). See
 * tst_fzsync_pair_wait_update().
 *
 * Returns a non-zero value if the thread should continue otherwise the
 * calling thread should exit.
 */
static inline int tst_fzsync_pair_wait(struct tst_fzsync_pair *pair,
				       int *our_cntr, int *other_cntr)
{
	if (tst_atomic_inc(other_cntr) == INT_MAX) {
		/*
		 * We are about to break the invariant that the thread with
		 * the lowest count is in front of the other. So we must wait
		 * here to ensure the other thread has atleast reached the
		 * line above before doing that. If we are in rear position
		 * then our counter may already have been set to zero.
		 */
		while (tst_atomic_load(our_cntr) > 0
		       && tst_atomic_load(our_cntr) < INT_MAX
		       && !tst_atomic_load(&pair->exit))
			;

		tst_atomic_store(0, other_cntr);
		/*
		 * Once both counters have been set to zero the invariant
		 * is restored and we can continue.
		 */
		while (tst_atomic_load(our_cntr) > 1
		       && !tst_atomic_load(&pair->exit))
			;
	} else {
		/*
		 * If our counter is less than the other thread's we are ahead
		 * of it and need to wait.
		 */
		while (tst_atomic_load(our_cntr) < tst_atomic_load(other_cntr)
		       && !tst_atomic_load(&pair->exit))
			;
	}

	/* Only exit if we have done the same amount of work as the other thread */
	return !tst_atomic_load(&pair->exit) ||
	  tst_atomic_load(other_cntr) <= tst_atomic_load(our_cntr);
}

static inline int tst_fzsync_wait_a(struct tst_fzsync_pair *pair)
{
	return tst_fzsync_pair_wait(pair, &pair->a_cntr, &pair->b_cntr);
}

static inline int tst_fzsync_wait_b(struct tst_fzsync_pair *pair)
{
	return tst_fzsync_pair_wait(pair, &pair->b_cntr, &pair->a_cntr);
}

/**
 * tst_fzsync_pair_wait_update_{a,b} - Wait and then recalculate
 *
 * This allows you to have two long running threads which wait for each other
 * every iteration. So each thread will exit this function at approximately
 * the same time. It also updates the delay values in a thread safe manner.
 *
 * You must call this function in both threads the same number of times each
 * iteration. So a call in one thread must match with a call in the
 * other. Make sure that calls to tst_fzsync_pair_wait() and
 * tst_fzsync_pair_wait_update() happen in the same order in each thread. That
 * is, make sure that a call to tst_fzsync_pair_wait_update_a() in one thread
 * corresponds to a call to tst_fzsync_pair_wait_update_b() in the other.
 *
 * Returns a non-zero value if the calling thread should continue to loop. If
 * it returns zero then tst_fzsync_exit() has been called and you must exit
 * the thread.
 */
static inline int tst_fzsync_wait_update_a(struct tst_fzsync_pair *pair)
{
	static int loop_index;

	tst_fzsync_pair_wait(pair, &pair->a_cntr, &pair->b_cntr);
	loop_index++;
	tst_fzsync_pair_update(loop_index, pair);
	return tst_fzsync_pair_wait(pair, &pair->a_cntr, &pair->b_cntr);
}

static inline int tst_fzsync_wait_update_b(struct tst_fzsync_pair *pair)
{
	tst_fzsync_pair_wait(pair, &pair->b_cntr, &pair->a_cntr);
	return tst_fzsync_pair_wait(pair, &pair->b_cntr, &pair->a_cntr);
}

/**
 * tst_fzsync_pair_exit - Signal that the other thread should exit
 *
 * Causes tst_fzsync_pair_wait() and tst_fzsync_pair_wait_update() to return
 * 0.
 */
static inline void tst_fzsync_pair_exit(struct tst_fzsync_pair *pair)
{
	tst_atomic_store(1, &pair->exit);
}
