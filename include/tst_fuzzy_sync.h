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
 * This library is intended to help reproduce race conditions while running in
 * a loop. You can use it to measure the time at which two functions are
 * called in different threads. Then calculate the average time gap between
 * the function calls and introduce a delay in one thread to synchronise the
 * calls.
 *
 * It is called 'fuzzy' synchronisation because the time gap will naturally vary
 * due to environmental factors. It is not a 'hard' synchronisation mechanism
 * like lockstepping.
 *
 * For a usage example see testcases/cve/cve-2017-2671.c
 */

#include <sys/time.h>
#include <time.h>

#ifndef CLOCK_MONOTONIC_RAW
# define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

/**
 * struct tst_fzsync_pair - the state of a two way synchronisation
 * @avg_diff: The average time difference over multiple iterations
 * @avg_diff_trgt: The desired average time difference, defaults to 0
 * @avg_alpha: The rate at which old diff samples are forgotten,
 *             defaults to 0.25
 * @a: The time at which call site A was last passed
 * @b: The time at which call site B was last passed
 * @delay: The size of the delay, positive to delay A, negative to delay B
 * @delay_inc: The step size of a delay increment, defaults to 10
 * @update_gap: The number of iterations between recalculating the delay.
 *              Defaults to 0xF and must be of the form $2^n - 1$
 *
 * This contains all the necessary state for synchronising two points A and
 * B. Where A is the time of an event in one process and B is the time of an
 * event in another process.
 */
struct tst_fzsync_pair {
	double avg_diff;
	double avg_diff_trgt;
	double avg_alpha;
	struct timespec a;
	struct timespec b;
	long delay;
	long delay_inc;
	int update_gap;
};

/**
 * TST_FZSYNC_PAIR_INIT - Default values for struct tst_fzysnc_pair
 */
#define TST_FZSYNC_PAIR_INIT {	\
	.avg_alpha = 0.25,	\
	.delay_inc = 10,	\
	.update_gap = 0xF	\
}

static void tst_fzsync_pair_info(struct tst_fzsync_pair *pair)
{
	tst_res(TINFO, "avg_diff = %.5gns, delay = %05ld loops",
		pair->avg_diff, pair->delay);
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
static inline double tst_exp_moving_avg(double alpha, long sample,
					double prev_avg)
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
	double target = pair->avg_diff_trgt;
	double avg = pair->avg_diff;

	diff = pair->a.tv_nsec - pair->b.tv_nsec;
	avg = tst_exp_moving_avg(pair->avg_alpha, diff, avg);

	if (!(loop_index & pair->update_gap)) {
		if (avg > target)
			pair->delay -= inc;
		else if (avg < target)
			pair->delay += inc;
	}

	pair->avg_diff = avg;
}
