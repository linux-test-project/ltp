/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008, 2009
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *     pi_perf.c
 *
 * DESCRIPTION
 *     Create a scenario with one high, one low and several
 *     medium priority threads. Low priority thread holds a PI lock, high
 *     priority thread later tries to grab it. The test measures the maximum
 *     amount of time the high priority thread has to wait before it gets
 *     the lock. This time should be bound by the duration for which low
 *     priority thread holds the lock
 *
 * USAGE:
 *     Use run_auto.sh script in current directory to build and run test.
 *     Use "-j" to enable jvm simulator.
 *
 * AUTHOR
 *     Author: Sripathi Kodi <sripathik@in.ibm.com>
 *
 * HISTORY
 *     2007-Nov-20:    Initial version by Sripathi Kodi <sripathik@in.ibm.com>
 *     2009-Jul-03:    Pass criteria corrected by Sripathi Kodi
 *						      <sripathik@in.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define LOWPRIO 30
#define HIGHPRIO 40
#define BUSYPRIO 35

#define DEF_LOW_WORK_MS 6
#define DEF_HIGH_WORK_MS 1
#define DEF_BUSY_WORK_MS 6
#define DEF_ITERATIONS 100

#define HIST_BUCKETS 100
#define THRESHOLD 200		/* microseconds */

pthread_barrier_t bar1, bar2;
pthread_mutex_t lock;

static int end = 0;

static unsigned int iterations = DEF_ITERATIONS;
static unsigned int low_work_time = DEF_LOW_WORK_MS;
static unsigned int high_work_time = DEF_HIGH_WORK_MS;
static unsigned int busy_work_time;
static int num_busy = -1;

nsec_t low_unlock, max_pi_delay;

stats_container_t low_dat, cpu_delay_dat;
stats_container_t cpu_delay_hist;
stats_quantiles_t cpu_delay_quantiles;
stats_record_t rec;

void usage(void)
{
	rt_help();
	printf("pi_perf_test specific options:\n");
	printf
	    ("  -nNUMBER   Number of busy threads. Default = number of cpus\n");
	printf("  -iNUMBER   Number of iterations. Default = %d\n",
	       DEF_ITERATIONS);
	printf("  -tPERIOD   Duration of work. Number of ms.\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'i':
		iterations = atoi(v);
		break;
	case 'n':
		num_busy = atoi(v);
		break;
	case 'w':
		low_work_time = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *busy_thread(void *arg)
{
	struct thread *thr = (struct thread *)arg;

	printf("Busy %ld started\n", (long)thr->arg);

	while (!end) {
		/* Wait for all threads to reach barrier wait */
		pthread_barrier_wait(&bar1);
		busy_work_ms(busy_work_time);
		/* Wait for all threads to finish this iteration */
		pthread_barrier_wait(&bar2);
	}
	return NULL;
}

void *low_prio_thread(void *arg)
{
	nsec_t low_start, low_hold;
	unsigned int i;

	stats_container_init(&low_dat, iterations);

	printf("Low prio thread started\n");

	for (i = 0; i < iterations; i++) {
		pthread_mutex_lock(&lock);
		/* Wait for all threads to reach barrier wait.
		   Since we already own the mutex, high prio
		   thread will boost our priority.
		 */
		pthread_barrier_wait(&bar1);

		low_start = rt_gettime();
		busy_work_ms(low_work_time);
		low_unlock = rt_gettime();
		low_hold = low_unlock - low_start;

		pthread_mutex_unlock(&lock);

		rec.x = i;
		rec.y = low_hold / NS_PER_US;
		stats_container_append(&low_dat, rec);

		if (i == iterations - 1)
			end = 1;

		/* Wait for all threads to finish this iteration */
		pthread_barrier_wait(&bar2);
	}

	return NULL;
}

void *high_prio_thread(void *arg)
{
	nsec_t high_start, high_end, high_get_lock;
	unsigned int i;

	stats_container_init(&cpu_delay_dat, iterations);
	stats_container_init(&cpu_delay_hist, HIST_BUCKETS);
	stats_quantiles_init(&cpu_delay_quantiles, (int)log10(iterations));

	printf("High prio thread started\n");

	for (i = 0; i < iterations; i++) {
		/* Wait for all threads to reach barrier wait. When
		   woken up, low prio thread will own the mutex
		 */
		pthread_barrier_wait(&bar1);

		high_start = rt_gettime();
		pthread_mutex_lock(&lock);
		high_end = rt_gettime();
		high_get_lock = high_end - low_unlock;

		busy_work_ms(high_work_time);
		pthread_mutex_unlock(&lock);

		rec.x = i;
		rec.y = high_get_lock / NS_PER_US;
		stats_container_append(&cpu_delay_dat, rec);

		/* Wait for all threads to finish this iteration */
		pthread_barrier_wait(&bar2);
	}

	stats_hist(&cpu_delay_hist, &cpu_delay_dat);
	stats_container_save("samples", "pi_perf Latency Scatter Plot",
			     "Iteration", "Latency (us)", &cpu_delay_dat,
			     "points");
	stats_container_save("hist", "pi_perf Latency Histogram",
			     "Latency (us)", "Samples", &cpu_delay_hist,
			     "steps");

	printf
	    ("Time taken for high prio thread to get the lock once released by low prio thread\n");
	printf("Min delay = %ld us\n", stats_min(&cpu_delay_dat));
	printf("Max delay = %ld us\n", stats_max(&cpu_delay_dat));
	printf("Average delay = %4.2f us\n", stats_avg(&cpu_delay_dat));
	printf("Standard Deviation = %4.2f us\n", stats_stddev(&cpu_delay_dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&cpu_delay_dat, &cpu_delay_quantiles);
	stats_quantiles_print(&cpu_delay_quantiles);

	max_pi_delay = stats_max(&cpu_delay_dat);

	return NULL;
}

int main(int argc, char *argv[])
{
	long i;
	int ret;
	setup();

	pass_criteria = THRESHOLD;
	rt_init("hi:n:w:", parse_args, argc, argv);

	if (iterations < 100) {
		printf("Number of iterations cannot be less than 100\n");
		exit(1);
	}

	busy_work_time = low_work_time;
	if (num_busy == -1) {
		/* Number of busy threads = No. of CPUs */
		num_busy = sysconf(_SC_NPROCESSORS_ONLN);
	}

	if ((ret = pthread_barrier_init(&bar1, NULL, (num_busy + 2)))) {
		printf("pthread_barrier_init failed: %s\n", strerror(ret));
		exit(ret);
	}
	if ((ret = pthread_barrier_init(&bar2, NULL, (num_busy + 2)))) {
		printf("pthread_barrier_init failed: %s\n", strerror(ret));
		exit(ret);
	}

	init_pi_mutex(&lock);

	if ((ret = create_fifo_thread(low_prio_thread, NULL, LOWPRIO)) < 0)
		exit(ret);
	if ((ret =
	     create_fifo_thread(high_prio_thread, NULL, HIGHPRIO)) < 0)
		exit(ret);

	for (i = 0; i < num_busy; i++) {
		if ((ret =
		     create_fifo_thread(busy_thread, (void *)i, BUSYPRIO)) < 0)
			exit(ret);
	}

	join_threads();
	printf("Criteria: High prio lock wait time < "
	       "(Low prio lock held time + %d us)\n", (int)pass_criteria);

	ret = 0;
	if (max_pi_delay > pass_criteria)
		ret = 1;

	printf("Result: %s\n", ret ? "FAIL" : "PASS");
	return ret;
}
