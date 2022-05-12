/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2017-2018 Richard Palethorpe <rpalethorpe@suse.com>
 */
/**
 * @file tst_fuzzy_sync.h
 * Fuzzy Synchronisation - abbreviated to fzsync
 *
 * This library is intended to help reproduce race conditions by synchronising
 * two threads at a given place by marking the range a race may occur
 * in. Because the exact place where any race occurs is within the kernel,
 * and therefore impossible to mark accurately, the library may add randomised
 * delays to either thread in order to help find the exact race timing.
 *
 * Currently only two way races are explicitly supported, that is races
 * involving two threads or processes. We refer to the main test thread as
 * thread A and the child thread as thread B.
 *
 * In each thread you need a simple while- or for-loop which the tst_fzsync_*
 * functions are called in. In the simplest case thread A will look something
 * like:
 *
 * tst_fzsync_pair_reset(&pair, run_thread_b);
 * while (tst_fzsync_run_a(&pair)) {
 *	// Perform some setup which must happen before the race
 *	tst_fzsync_start_race_a(&pair);
 *	// Do some dodgy syscall
 *	tst_fzsync_end_race_a(&pair);
 * }
 *
 * Then in thread B (run_thread_b):
 *
 * while (tst_fzsync_run_b(&pair)) {
 *	tst_fzsync_start_race_b(&pair);
 *	// Do something which can race with the dodgy syscall in A
 *	tst_fzsync_end_race_b(&pair)
 * }
 *
 * The calls to tst_fzsync_start/end_race and tst_fzsync_run_a/b block (at
 * least) until both threads have enter them. These functions can only be
 * called once for each iteration, but further synchronisation points can be
 * added by calling tst_fzsync_wait_a() and tst_fzsync_wait_b() in each
 * thread.
 *
 * The execution of the loops in threads A and B are bounded by both iteration
 * count and time. A slow machine is likely to be limited by time and a fast
 * one by iteration count. The user can use the -i parameter to run the test
 * multiple times or LTP_TIMEOUT_MUL to give the test more time.
 *
 * It is possible to use the library just for tst_fzsync_pair_wait() to get a
 * basic spin wait. However if you are actually testing a race condition then
 * it is recommended to use tst_fzsync_start_race_a/b even if the
 * randomisation is not needed. It provides some semantic information which
 * may be useful in the future.
 *
 * For a usage example see testcases/cve/cve-2016-7117.c or just run
 * 'git grep tst_fuzzy_sync.h'
 *
 * @sa tst_fzsync_pair
 */

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "tst_atomic.h"
#include "tst_cpu.h"
#include "tst_timer.h"
#include "tst_safe_pthread.h"

#ifndef TST_FUZZY_SYNC_H__
#define TST_FUZZY_SYNC_H__

/* how much of exec time is sampling allowed to take */
#define SAMPLING_SLICE 0.5f

/** Some statistics for a variable */
struct tst_fzsync_stat {
	float avg;
	float avg_dev;
	float dev_ratio;
};

/**
 * The state of a two way synchronisation or race.
 *
 * This contains all the necessary state for approximately synchronising two
 * sections of code in different threads.
 *
 * Some of the fields can be configured before calling
 * tst_fzsync_pair_reset(), however this is mainly for debugging purposes. If
 * a test requires one of the parameters to be modified, we should consider
 * finding a way of automatically selecting an appropriate value at runtime.
 *
 * Internal fields should only be accessed by library functions.
 */
struct tst_fzsync_pair {
	/**
	 * The rate at which old diff samples are forgotten
	 *
	 * Defaults to 0.25.
	 */
	float avg_alpha;
	/** Internal; Thread A start time */
	struct timespec a_start;
	/** Internal; Thread B start time */
	struct timespec b_start;
	/** Internal; Thread A end time */
	struct timespec a_end;
	/** Internal; Thread B end time */
	struct timespec b_end;
	/** Internal; Avg. difference between a_start and b_start */
	struct tst_fzsync_stat diff_ss;
	/** Internal; Avg. difference between a_start and a_end */
	struct tst_fzsync_stat diff_sa;
	/** Internal; Avg. difference between b_start and b_end */
	struct tst_fzsync_stat diff_sb;
	/** Internal; Avg. difference between a_end and b_end */
	struct tst_fzsync_stat diff_ab;
	/** Internal; Number of spins while waiting for the slower thread */
	int spins;
	struct tst_fzsync_stat spins_avg;
	/**
	 * Internal; Number of spins to use in the delay.
	 *
	 * A negative value delays thread A and a positive delays thread B.
	 */
	int delay;
	int delay_bias;
	/**
	 *  Internal; The number of samples left or the sampling state.
	 *
	 *  A positive value is the number of remaining mandatory
	 *  samples. Zero or a negative indicate some other state.
	 */
	int sampling;
	/**
	 * The Minimum number of statistical samples which must be collected.
	 *
	 * The minimum number of iterations which must be performed before a
	 * random delay can be calculated. Defaults to 1024.
	 */
	int min_samples;
	/**
	 * The maximum allowed proportional average deviation.
	 *
	 * A value in the range (0, 1) which gives the maximum average
	 * deviation which must be attained before random delays can be
	 * calculated.
	 *
	 * It is a ratio of (average_deviation / total_time). The default is
	 * 0.1, so this allows an average deviation of at most 10%.
	 */
	float max_dev_ratio;

	/** Internal; Atomic counter used by fzsync_pair_wait() */
	int a_cntr;
	/** Internal; Atomic counter used by fzsync_pair_wait() */
	int b_cntr;
	/** Internal; Used by tst_fzsync_pair_exit() and fzsync_pair_wait() */
	int exit;
	/** Internal; The test time remaining on tst_fzsync_pair_reset() */
	float exec_time_start;
	/**
	 * The maximum number of iterations to execute during the test
	 *
	 * Defaults to a large number, but not too large.
	 */
	int exec_loops;
	/** Internal; The current loop index  */
	int exec_loop;
	/** Internal; The second thread or 0 */
	pthread_t thread_b;
	/**
	 * The flag indicates single core machines or not
	 *
	 * If running on single core machines, it would take considerable
	 * amount of time to run fuzzy sync library.
	 * Thus call sched_yield to give up cpu to decrease the test time.
	 */
	bool yield_in_wait;

};

#define CHK(param, low, hi, def) do {					      \
	pair->param = (pair->param ? pair->param : def);		      \
	if (pair->param < low)						      \
		tst_brk(TBROK, #param " is less than the lower bound " #low); \
	if (pair->param > hi)						      \
		tst_brk(TBROK, #param " is more than the upper bound " #hi);  \
	} while (0)
/**
 * Ensures that any Fuzzy Sync parameters are properly set
 *
 * @relates tst_fzsync_pair
 *
 * Usually called from the setup function, it sets default parameter values or
 * validates any existing non-defaults.
 *
 * @sa tst_fzsync_pair_reset()
 */
static inline void tst_fzsync_pair_init(struct tst_fzsync_pair *pair)
{
	CHK(avg_alpha, 0, 1, 0.25);
	CHK(min_samples, 20, INT_MAX, 1024);
	CHK(max_dev_ratio, 0, 1, 0.1);
	CHK(exec_loops, 20, INT_MAX, 3000000);

	if (tst_ncpus_available() <= 1)
		pair->yield_in_wait = 1;
}
#undef CHK

/**
 * Exit and join thread B if necessary.
 *
 * @relates tst_fzsync_pair
 *
 * Call this from your cleanup function.
 */
static inline void tst_fzsync_pair_cleanup(struct tst_fzsync_pair *pair)
{
	if (pair->thread_b) {
		/* Revoke thread B if parent hits accidental break */
		if (!pair->exit)
			tst_atomic_store(1, &pair->exit);
		SAFE_PTHREAD_JOIN(pair->thread_b, NULL);
		pair->thread_b = 0;
	}
}

/**
 * Zero some stat fields
 *
 * @relates tst_fzsync_stat
 */
static inline void tst_init_stat(struct tst_fzsync_stat *s)
{
	s->avg = 0;
	s->avg_dev = 0;
}

/**
 * Reset or initialise fzsync.
 *
 * @relates tst_fzsync_pair
 * @param pair The state structure initialised with TST_FZSYNC_PAIR_INIT.
 * @param run_b The function defining thread B or NULL.
 *
 * Call this from your main test function (thread A), just before entering the
 * main loop. It will (re)set any variables needed by fzsync and (re)start
 * thread B using the function provided.
 *
 * If you need to use fork or clone to start the second thread/process then
 * you can pass NULL to run_b and handle starting and stopping thread B
 * yourself. You may need to place tst_fzsync_pair in some shared memory as
 * well.
 *
 * @sa tst_fzsync_pair_init()
 */
static inline void tst_fzsync_pair_reset(struct tst_fzsync_pair *pair,
				  void *(*run_b)(void *))
{
	tst_fzsync_pair_cleanup(pair);

	tst_init_stat(&pair->diff_ss);
	tst_init_stat(&pair->diff_sa);
	tst_init_stat(&pair->diff_sb);
	tst_init_stat(&pair->diff_ab);
	tst_init_stat(&pair->spins_avg);
	pair->delay = 0;
	pair->delay_bias = 0;
	pair->sampling = pair->min_samples;

	pair->exec_loop = 0;

	pair->a_cntr = 0;
	pair->b_cntr = 0;
	pair->exit = 0;
	if (run_b)
		SAFE_PTHREAD_CREATE(&pair->thread_b, 0, run_b, 0);

	pair->exec_time_start = (float)tst_remaining_runtime();
}

/**
 * Print stat
 *
 * @relates tst_fzsync_stat
 */
static inline void tst_fzsync_stat_info(struct tst_fzsync_stat stat,
					char *unit, char *name)
{
	tst_res(TINFO,
		"%1$-17s: { avg = %3$5.0f%2$s, avg_dev = %4$5.0f%2$s, dev_ratio = %5$.2f }",
		name, unit, stat.avg, stat.avg_dev, stat.dev_ratio);
}

/**
 * Print some synchronisation statistics
 *
 * @relates tst_fzsync_pair
 */
static inline void tst_fzsync_pair_info(struct tst_fzsync_pair *pair)
{
	tst_res(TINFO, "loop = %d, delay_bias = %d",
		pair->exec_loop, pair->delay_bias);
	tst_fzsync_stat_info(pair->diff_ss, "ns", "start_a - start_b");
	tst_fzsync_stat_info(pair->diff_sa, "ns", "end_a - start_a");
	tst_fzsync_stat_info(pair->diff_sb, "ns", "end_b - start_b");
	tst_fzsync_stat_info(pair->diff_ab, "ns", "end_a - end_b");
	tst_fzsync_stat_info(pair->spins_avg, "  ", "spins");
}

/** Wraps clock_gettime */
static inline void tst_fzsync_time(struct timespec *t)
{
#ifdef CLOCK_MONOTONIC_RAW
	clock_gettime(CLOCK_MONOTONIC_RAW, t);
#else
	clock_gettime(CLOCK_MONOTONIC, t);
#endif
}

/**
 * Exponential moving average
 *
 * @param alpha The preference for recent samples over old ones.
 * @param sample The current sample
 * @param prev_avg The average of the all the previous samples
 *
 * @return The average including the current sample.
 */
static inline float tst_exp_moving_avg(float alpha,
					float sample,
					float prev_avg)
{
	return alpha * sample + (1.0 - alpha) * prev_avg;
}

/**
 * Update a stat with a new sample
 *
 * @relates tst_fzsync_stat
 */
static inline void tst_upd_stat(struct tst_fzsync_stat *s,
				 float alpha,
				 float sample)
{
	s->avg = tst_exp_moving_avg(alpha, sample, s->avg);
	s->avg_dev = tst_exp_moving_avg(alpha,
					fabs(s->avg - sample), s->avg_dev);
	s->dev_ratio = fabs(s->avg ? s->avg_dev / s->avg : 0);
}

/**
 * Update a stat with a new diff sample
 *
 * @relates tst_fzsync_stat
 */
static inline void tst_upd_diff_stat(struct tst_fzsync_stat *s,
				     float alpha,
				     struct timespec t1,
				     struct timespec t2)
{
	tst_upd_stat(s, alpha, tst_timespec_diff_ns(t1, t2));
}

/**
 * Calculate various statistics and the delay
 *
 * This function helps create the fuzz in fuzzy sync. Imagine we have the
 * following timelines in threads A and B:
 *
 *  start_race_a
 *      ^                    end_race_a (a)
 *      |                        ^
 *      |                        |
 *  - --+------------------------+-- - -
 *      |        Syscall A       |                 Thread A
 *  - --+------------------------+-- - -
 *  - --+----------------+-------+-- - -
 *      |   Syscall B    | spin  |                 Thread B
 *  - --+----------------+-------+-- - -
 *      |                |
 *      ^                ^
 *  start_race_b     end_race_b
 *
 * Here we have synchronised the calls to syscall A and B with start_race_{a,
 * b} so that they happen at approximately the same time in threads A and
 * B. If the race condition occurs during the entry code for these two
 * functions then we will quickly hit it. If it occurs during the exit code of
 * B and mid way through A, then we will quickly hit it.
 *
 * However if the exit paths of A and B need to be aligned and (end_race_a -
 * end_race_b) is large relative to the variation in call times, the
 * probability of hitting the race condition is close to zero. To solve this
 * scenario (and others) a randomised delay is introduced before the syscalls
 * in A and B. Given enough time the following should happen where the exit
 * paths are now synchronised:
 *
 *  start_race_a
 *      ^                    end_race_a (a)
 *      |                        ^
 *      |                        |
 *  - --+------------------------+-- - -
 *      |        Syscall A       |                 Thread A
 *  - --+------------------------+-- - -
 *  - --+-------+----------------+-- - -
 *      | delay |   Syscall B    |                 Thread B
 *  - --+-------+----------------+-- - -
 *      |                        |
 *      ^                        ^
 *  start_race_b             end_race_b
 *
 * The delay is not introduced immediately and the delay range is only
 * calculated once the average relative deviation has dropped below some
 * percentage of the total time.
 *
 * The delay range is chosen so that any point in Syscall A could be
 * synchronised with any point in Syscall B using a value from the
 * range. Because the delay range may be too large for a linear search, we use
 * an evenly distributed random function to pick a value from it.
 *
 * The delay range goes from positive to negative. A negative delay will delay
 * thread A and a positive one will delay thread B. The range is bounded by
 * the point where the entry code to Syscall A is synchronised with the exit
 * to Syscall B and the entry code to Syscall B is synchronised with the exit
 * of A.
 *
 * In order to calculate the lower bound (the max delay of A) we can simply
 * negate the execution time of Syscall B and convert it to a spin count. For
 * the upper bound (the max delay of B), we just take the execution time of A
 * and convert it to a spin count.
 *
 * In order to calculate spin count we need to know approximately how long a
 * spin takes and divide the delay time with it. We find this by first
 * counting how many spins one thread spends waiting for the other during
 * end_race[1]. We also know when each syscall exits so we can take the
 * difference between the exit times and divide it with the number of spins
 * spent waiting.
 *
 * All the times and counts we use in the calculation are averaged over a
 * variable number of iterations. There is an initial sampling period where we
 * simply collect time and count samples then calculate their averages. When a
 * minimum number of samples have been collected, and if the average deviation
 * is below some proportion of the average sample magnitude, then the sampling
 * period is ended. On all further iterations a random delay is calculated and
 * applied, but the averages are not updated.
 *
 * [1] This assumes there is always a significant difference. The algorithm
 * may fail to introduce a delay (when one is needed) in situations where
 * Syscall A and B finish at approximately the same time.
 *
 * @relates tst_fzsync_pair
 */
static inline void tst_fzsync_pair_update(struct tst_fzsync_pair *pair)
{
	float alpha = pair->avg_alpha;
	float per_spin_time, time_delay;
	float max_dev = pair->max_dev_ratio;
	int over_max_dev;

	pair->delay = pair->delay_bias;

	over_max_dev = pair->diff_ss.dev_ratio > max_dev
		|| pair->diff_sa.dev_ratio > max_dev
		|| pair->diff_sb.dev_ratio > max_dev
		|| pair->diff_ab.dev_ratio > max_dev
		|| pair->spins_avg.dev_ratio > max_dev;

	if (pair->sampling > 0 || over_max_dev) {
		tst_upd_diff_stat(&pair->diff_ss, alpha,
				  pair->a_start, pair->b_start);
		tst_upd_diff_stat(&pair->diff_sa, alpha,
				  pair->a_end, pair->a_start);
		tst_upd_diff_stat(&pair->diff_sb, alpha,
				  pair->b_end, pair->b_start);
		tst_upd_diff_stat(&pair->diff_ab, alpha,
				  pair->a_end, pair->b_end);
		tst_upd_stat(&pair->spins_avg, alpha, pair->spins);
		if (pair->sampling > 0 && --pair->sampling == 0) {
			tst_res(TINFO, "Minimum sampling period ended");
			tst_fzsync_pair_info(pair);
		}
	} else if (fabsf(pair->diff_ab.avg) >= 1) {
		per_spin_time = fabsf(pair->diff_ab.avg) / MAX(pair->spins_avg.avg, 1.0f);
		time_delay = drand48() * (pair->diff_sa.avg + pair->diff_sb.avg)
			- pair->diff_sb.avg;
		pair->delay += (int)(1.1 * time_delay / per_spin_time);

		if (!pair->sampling) {
			tst_res(TINFO,
				"Reached deviation ratios < %.2f, introducing randomness",
				pair->max_dev_ratio);
			tst_res(TINFO, "Delay range is [%d, %d]",
				-(int)(pair->diff_sb.avg / per_spin_time) + pair->delay_bias,
				(int)(pair->diff_sa.avg / per_spin_time) + pair->delay_bias);
			tst_fzsync_pair_info(pair);
			pair->sampling = -1;
		}
	} else if (!pair->sampling) {
		tst_res(TWARN, "Can't calculate random delay");
		tst_fzsync_pair_info(pair);
		pair->sampling = -1;
	}

	pair->spins = 0;
}

/**
 * Wait for the other thread
 *
 * @relates tst_fzsync_pair
 * @param our_cntr The counter for the thread we are on
 * @param other_cntr The counter for the thread we are synchronising with
 * @param spins A pointer to the spin counter or NULL
 * @param exit Exit flag when we need to break out of the wait loop
 *
 * Used by tst_fzsync_pair_wait_a(), tst_fzsync_pair_wait_b(),
 * tst_fzsync_start_race_a(), etc. If the calling thread is ahead of the other
 * thread, then it will spin wait. Unlike pthread_barrier_wait it will never
 * use futex and can count the number of spins spent waiting.
 *
 * @return A non-zero value if the thread should continue otherwise the
 * calling thread should exit.
 */
static inline void tst_fzsync_pair_wait(int *our_cntr,
					int *other_cntr,
					int *spins,
					int *exit,
					bool yield_in_wait)
{
	if (tst_atomic_inc(other_cntr) == INT_MAX) {
		/*
		 * We are about to break the invariant that the thread with
		 * the lowest count is in front of the other. So we must wait
		 * here to ensure the other thread has at least reached the
		 * line above before doing that. If we are in rear position
		 * then our counter may already have been set to zero.
		 */
		if (yield_in_wait) {
			while (tst_atomic_load(our_cntr) > 0
			       && tst_atomic_load(our_cntr) < INT_MAX
			       && !tst_atomic_load(exit)) {
				if (spins)
					(*spins)++;

				sched_yield();
			}
		} else {
			while (tst_atomic_load(our_cntr) > 0
			       && tst_atomic_load(our_cntr) < INT_MAX
			       && !tst_atomic_load(exit)) {
				if (spins)
					(*spins)++;
			}
		}


		tst_atomic_store(0, other_cntr);
		/*
		 * Once both counters have been set to zero the invariant
		 * is restored and we can continue.
		 */
		if (yield_in_wait) {
			while (tst_atomic_load(our_cntr) > 1
			       && !tst_atomic_load(exit))
				sched_yield();
		} else {
			while (tst_atomic_load(our_cntr) > 1
			       && !tst_atomic_load(exit))
				;
		}
	} else {
		/*
		 * If our counter is less than the other thread's we are ahead
		 * of it and need to wait.
		 */
		if (yield_in_wait) {
			while (tst_atomic_load(our_cntr) <
			       tst_atomic_load(other_cntr)
			       && !tst_atomic_load(exit)) {
				if (spins)
					(*spins)++;
				sched_yield();
			}
		} else {
			while (tst_atomic_load(our_cntr) <
			       tst_atomic_load(other_cntr)
			       && !tst_atomic_load(exit)) {
				if (spins)
					(*spins)++;
			}
		}
	}
}

/**
 * Wait in thread A
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_pair_wait
 */
static inline void tst_fzsync_wait_a(struct tst_fzsync_pair *pair)
{
	tst_fzsync_pair_wait(&pair->a_cntr, &pair->b_cntr,
			     NULL, &pair->exit, pair->yield_in_wait);
}

/**
 * Wait in thread B
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_pair_wait
 */
static inline void tst_fzsync_wait_b(struct tst_fzsync_pair *pair)
{
	tst_fzsync_pair_wait(&pair->b_cntr, &pair->a_cntr,
			     NULL, &pair->exit, pair->yield_in_wait);
}

/**
 * Decide whether to continue running thread A
 *
 * @relates tst_fzsync_pair
 *
 * Checks some values and decides whether it is time to break the loop of
 * thread A.
 *
 * @return True to continue and false to break.
 * @sa tst_fzsync_run_a
 */
static inline int tst_fzsync_run_a(struct tst_fzsync_pair *pair)
{
	float rem_p = 1 - tst_remaining_runtime() / pair->exec_time_start;

	if ((SAMPLING_SLICE < rem_p) && (pair->sampling > 0)) {
		tst_res(TINFO, "Stopped sampling at %d (out of %d) samples, "
			"sampling time reached 50%% of the total time limit",
			pair->exec_loop, pair->min_samples);
		pair->sampling = 0;
		tst_fzsync_pair_info(pair);
	}

	if (rem_p >= 1) {
		tst_res(TINFO,
			"Exceeded execution time, requesting exit");
		tst_atomic_store(1, &pair->exit);
	}

	if (++pair->exec_loop > pair->exec_loops) {
		tst_res(TINFO,
			"Exceeded execution loops, requesting exit");
		tst_atomic_store(1, &pair->exit);
	}

	tst_fzsync_wait_a(pair);

	if (pair->exit) {
		tst_fzsync_pair_cleanup(pair);
		return 0;
	}

	return 1;
}

/**
 * Decide whether to continue running thread B
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_run_a
 */
static inline int tst_fzsync_run_b(struct tst_fzsync_pair *pair)
{
	tst_fzsync_wait_b(pair);
	return !tst_atomic_load(&pair->exit);
}

/**
 * Marks the start of a race region in thread A
 *
 * @relates tst_fzsync_pair
 *
 * This should be placed just before performing whatever action can cause a
 * race condition. Usually it is placed just before a syscall and
 * tst_fzsync_end_race_a() is placed just afterwards.
 *
 * A corresponding call to tst_fzsync_start_race_b() should be made in thread
 * B.
 *
 * @return A non-zero value if the calling thread should continue to loop. If
 * it returns zero then tst_fzsync_exit() has been called and you must exit
 * the thread.
 *
 * @sa tst_fzsync_pair_update
 */
static inline void tst_fzsync_start_race_a(struct tst_fzsync_pair *pair)
{
	volatile int delay;

	tst_fzsync_pair_update(pair);

	tst_fzsync_wait_a(pair);

	delay = pair->delay;
	if (pair->yield_in_wait) {
		while (delay < 0) {
			sched_yield();
			delay++;
		}
	} else {
		while (delay < 0)
			delay++;
	}

	tst_fzsync_time(&pair->a_start);
}

/**
 * Marks the end of a race region in thread A
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_start_race_a
 */
static inline void tst_fzsync_end_race_a(struct tst_fzsync_pair *pair)
{
	tst_fzsync_time(&pair->a_end);
	tst_fzsync_pair_wait(&pair->a_cntr, &pair->b_cntr,
			     &pair->spins, &pair->exit, pair->yield_in_wait);
}

/**
 * Marks the start of a race region in thread B
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_start_race_a
 */
static inline void tst_fzsync_start_race_b(struct tst_fzsync_pair *pair)
{
	volatile int delay;

	tst_fzsync_wait_b(pair);

	delay = pair->delay;
	if (pair->yield_in_wait) {
		while (delay > 0) {
			sched_yield();
			delay--;
		}
	} else {
		while (delay > 0)
			delay--;
	}

	tst_fzsync_time(&pair->b_start);
}

/**
 * Marks the end of a race region in thread B
 *
 * @relates tst_fzsync_pair
 * @sa tst_fzsync_start_race_a
 */
static inline void tst_fzsync_end_race_b(struct tst_fzsync_pair *pair)
{
	tst_fzsync_time(&pair->b_end);
	tst_fzsync_pair_wait(&pair->b_cntr, &pair->a_cntr,
			     &pair->spins, &pair->exit, pair->yield_in_wait);
}

/**
 * Add some amount to the delay bias
 *
 * @relates tst_fzsync_pair
 * @param change The amount to add, can be negative
 *
 * A positive change delays thread B and a negative one delays thread
 * A.
 *
 * It is intended to be used in tests where the time taken by syscall A and/or
 * B are significantly affected by their chronological order. To the extent
 * that the delay range will not include the correct values if too many of the
 * initial samples are taken when the syscalls (or operations within the
 * syscalls) happen in the wrong order.
 *
 * An example of this is cve/cve-2016-7117.c where a call to close() is racing
 * with a call to recvmmsg(). If close() happens before recvmmsg() has chance
 * to check if the file descriptor is open then recvmmsg() completes very
 * quickly. If the call to close() happens once recvmmsg() has already checked
 * the descriptor it takes much longer. The sample where recvmmsg() completes
 * quickly is essentially invalid for our purposes. The test uses the simple
 * heuristic of whether recvmmsg() returns EBADF, to decide if it should call
 * tst_fzsync_pair_add_bias() to further delay syscall B.
 */
static inline void tst_fzsync_pair_add_bias(struct tst_fzsync_pair *pair, int change)
{
	if (pair->sampling > 0)
		pair->delay_bias += change;
}

#endif /* TST_FUZZY_SYNC_H__ */
