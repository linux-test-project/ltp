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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *     hrtimer-prio.c
 *
 * DESCRIPTION
 *     Test the latency of hrtimers under rt load.
 *     The busy_threads should run at a priority higher than the system
 *     softirq_hrtimer, but lower than the timer_thread.  The timer_thread
 *     measure the time it takes to return from a nanosleep call.  If the
 *     lower priority threads can increase the latency of the higher
 *     priority thread, it is considered a failure.
 *
 * USAGE:
 *     Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-Aug-08:      Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define DEF_MED_PRIO 60		// (softirqd-hrtimer,98)
#define DEF_ITERATIONS 10000
#define HIST_BUCKETS 100
#define DEF_BUSY_TIME 10	// Duration of busy work in milliseconds
#define DEF_SLEEP_TIME 10000	// Duration of nanosleep in nanoseconds
#define DEF_CRITERIA 10		// maximum timer latency in microseconds

static int med_prio = DEF_MED_PRIO;
static int high_prio;
static int busy_time = DEF_BUSY_TIME;
static int iterations = DEF_ITERATIONS;
static int busy_threads;

static stats_container_t dat;
static stats_record_t rec;
static atomic_t busy_threads_started;
static unsigned long min_delta;
static unsigned long max_delta;

void usage(void)
{
	rt_help();
	printf("hrtimer-prio specific options:\n");
	printf("  -t#	   #:busy work time in ms, defaults to %d ms\n",
	       DEF_BUSY_TIME);
	printf("  -i#	   #:number of iterations, defaults to %d\n",
	       DEF_ITERATIONS);
	printf("  -n#	   #:number of busy threads, defaults to NR_CPUS*2\n");
	printf
	    ("  -f#	   #:rt fifo priority of busy threads (1,98), defaults to %d\n",
	     DEF_MED_PRIO);
	printf
	    ("  -m#	   #:maximum timer latency in microseconds, defaults to %d\n",
	     DEF_CRITERIA);
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 't':
		busy_time = atoi(v);
		break;
	case 'n':
		busy_threads = atoi(v);
		break;
	case 'f':
		med_prio = MIN(atoi(v), 98);
		break;
	case 'i':
		iterations = atoi(v);
		if (iterations < 100) {
			fprintf(stderr,
				"Number of iterations cannot be less than 100.\n");
			exit(1);
		}
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *busy_thread(void *thread)
{
	atomic_inc(&busy_threads_started);
	while (1) {
		busy_work_ms(busy_time);
		sched_yield();
	}
	return NULL;
}

void *timer_thread(void *thread)
{
	int i;
	nsec_t start, end;
	unsigned long delta_us;
	while (atomic_get(&busy_threads_started) < busy_threads) {
		rt_nanosleep(10000);
	}
	printf("All Busy Threads started, commencing test\n");	// FIXME: use debug infrastructure
	max_delta = 0;
	for (i = 0; i < iterations; i++) {
		start = rt_gettime();
		rt_nanosleep(DEF_SLEEP_TIME);
		end = rt_gettime();
		delta_us =
		    ((unsigned long)(end - start) - DEF_SLEEP_TIME) / NS_PER_US;
		rec.x = i;
		rec.y = delta_us;
		stats_container_append(&dat, rec);
		max_delta = MAX(max_delta, delta_us);
		min_delta = (i == 0) ? delta_us : MIN(min_delta, delta_us);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret = 1;
	int b;
	float avg_delta;
	int t_id;
	setup();
	busy_threads = 2 * sysconf(_SC_NPROCESSORS_ONLN);	// default busy_threads
	pass_criteria = DEF_CRITERIA;
	rt_init("f:i:jhn:t:", parse_args, argc, argv);
	high_prio = med_prio + 1;

	// Set main()'s prio to one above the timer_thread so it is sure to not
	// be starved
	if (set_priority(high_prio + 1) < 0) {
		printf("Failed to set main()'s priority to %d\n",
		       high_prio + 1);
		exit(1);
	}

	printf("\n-------------------------------------------\n");
	printf("High Resolution Timer Priority (Starvation)\n");
	printf("-------------------------------------------\n\n");
	printf("Running %d iterations\n", iterations);
	printf("Running with %d busy threads\n", busy_threads);
	printf("Busy thread work time: %d\n", busy_time);
	printf("Busy thread priority: %d\n", med_prio);
	printf("Timer thread priority: %d\n", high_prio);

	stats_container_t hist;
	stats_quantiles_t quantiles;
	if (stats_container_init(&dat, iterations)) {
		printf("Cannot init stat containers for dat\n");
		exit(1);
	}
	if (stats_container_init(&hist, HIST_BUCKETS)) {
		printf("Cannot init stat containers for hist\n");
		exit(1);
	}
	if (stats_quantiles_init(&quantiles, (int)log10(iterations))) {
		printf("Cannot init stat quantiles\n");
		exit(1);
	}

	t_id = create_fifo_thread(timer_thread, NULL, high_prio);
	if (t_id == -1) {
		printf("Failed to create timer thread\n");
		exit(1);
	}
	for (b = 0; b < busy_threads; b++) {
		if (create_fifo_thread(busy_thread, NULL, med_prio) < 0) {
			printf("Failed to create a busy thread\n");
			exit(1);
		}
	}
	join_thread(t_id);

	avg_delta = stats_avg(&dat);
	stats_hist(&hist, &dat);
	stats_container_save("samples",
			     "High Resolution Timer Latency Scatter Plot",
			     "Iteration", "Latency (us)", &dat, "points");
	stats_container_save("hist", "High Resolution Timer Latency Histogram",
			     "Latency (us)", "Samples", &hist, "steps");

	if (max_delta <= pass_criteria)
		ret = 0;

	printf("Minimum: %ld us\n", min_delta);
	printf("Maximum: %ld us\n", max_delta);
	printf("Average: %f us\n", avg_delta);
	printf("Standard Deviation: %f\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
	printf("\nCriteria: Maximum wakeup latency < %lu us\n",
	       (unsigned long)pass_criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return ret;
}
