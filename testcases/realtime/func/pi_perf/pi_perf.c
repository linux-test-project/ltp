/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *     Compilation: gcc -O2 -g -D_GNU_SOURCE -I/usr/include/nptl -I../../include
 *     -L/usr/lib/nptl -lpthread -lrt -lm pi_perf.c -o pi_perf
 *
 * AUTHOR
 *     Author: Sripathi Kodi <sripathik@in.ibm.com>
 *
 * HISTORY
 *     2007-Nov-20:    Initial version by Sripathi Kodi <sripathik@in.ibm.com>
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
#define THRESHOLD 200 /* microseconds */

pthread_barrier_t bar1, bar2;
pthread_mutex_t lock;

static int end = 0;

static unsigned int iterations = DEF_ITERATIONS;
static unsigned int low_work_time = DEF_LOW_WORK_MS;
static unsigned int high_work_time = DEF_HIGH_WORK_MS;
static unsigned int busy_work_time;
static int num_busy = -1;

stats_container_t high_dat, low_dat, wait_dat;
stats_container_t wait_hist;
stats_quantiles_t wait_quantiles;

void usage(void)
{
	rt_help();
	printf("pi_perf_test specific options:\n");
	printf("  -nNUMBER   Number of busy threads. Default = number of cpus\n");
	printf("  -iNUMBER   Number of iterations. Default = %d\n", DEF_ITERATIONS);
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

void * busy_thread(void *arg)
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

void * low_prio_thread(void *arg)
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
		low_hold = rt_gettime() - low_start;

		pthread_mutex_unlock(&lock);

		low_dat.records[i].x = i;
		low_dat.records[i].y = low_hold / NS_PER_US;

		if (i == iterations-1)
			end = 1;

		/* Wait for all threads to finish this iteration */
		pthread_barrier_wait(&bar2);
	}

	return NULL;
}


void * high_prio_thread(void *arg)
{
	nsec_t high_start, high_spent;
	unsigned int i;

	stats_container_init(&high_dat, iterations);
	stats_container_init(&wait_dat, iterations);
	stats_container_init(&wait_hist, HIST_BUCKETS);
	stats_quantiles_init(&wait_quantiles, (int)log10(iterations));

	printf("High prio thread started\n");

	for (i = 0; i < iterations; i++) {
		/* Wait for all threads to reach barrier wait. When
		   woken up, low prio thread will own the mutex
		 */
		pthread_barrier_wait(&bar1);

		high_start = rt_gettime();
		pthread_mutex_lock(&lock);
		high_spent = rt_gettime() - high_start;

		busy_work_ms(high_work_time);
		pthread_mutex_unlock(&lock);

		high_dat.records[i].x = i;
		high_dat.records[i].y = high_spent / NS_PER_US;
		wait_dat.records[i].x = i;
		wait_dat.records[i].y = high_dat.records[i].y - low_dat.records[i].y;

		/* Wait for all threads to finish this iteration */
		pthread_barrier_wait(&bar2);
	}
	stats_quantiles_calc(&wait_dat, &wait_quantiles);
	stats_hist(&wait_hist, &wait_dat);

	printf("Min wait time = %ld us\n", stats_min(&wait_dat));
	printf("Max wait time = %ld us\n", stats_max(&wait_dat));
	printf("Average wait time = %4.2f us\n", stats_avg(&wait_dat));
	printf("Standard Deviation = %4.2f us\n", stats_stddev(&wait_dat));
	printf("Quantiles:\n");
	stats_quantiles_print(&wait_quantiles);
	stats_container_save("samples", "pi_perf Latency Scatter Plot",
		"Iteration", "Latency (us)", &wait_dat, "points");
	stats_container_save("hist", "pi_perf Latency Histogram",
		"Latency (us)", "Samples", &wait_hist, "steps");

	return NULL;
}


int main(int argc, char *argv[])
{
	long i;
	int ret;
	setup();

	rt_init("hi:n:w:", parse_args, argc, argv);

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


	if ((ret = create_fifo_thread(low_prio_thread, (void *)0, LOWPRIO)) < 0)
		exit(ret);
	if ((ret = create_fifo_thread(high_prio_thread, (void *)0, HIGHPRIO)) < 0)
		exit(ret);

	for (i = 0; i < num_busy; i++) {
		if ((ret = create_fifo_thread(busy_thread, (void *)i, BUSYPRIO)) < 0)
			exit(ret);
	}

	join_threads();
	printf("Low prio lock held time (min) = %ld us\n", stats_min(&low_dat));
	printf("High prio lock wait time (max) = %ld us\n", stats_max(&high_dat));
	printf("Criteria: High prio lock wait time < "
			"(Low prio lock held time + %d us)\n", THRESHOLD);

	ret = 0;
	if (stats_max(&high_dat) > stats_min(&low_dat) + THRESHOLD)
		ret = 1;

	printf("Result: %s\n", ret ? "FAIL" : "PASS");
	return ret;
}
