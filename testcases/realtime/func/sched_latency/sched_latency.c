/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006-2008
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
 *      sched_latency.c
 *
 * DESCRIPTION
 *	Measure the latency involved with periodic scheduling.
 *   Steps:
 *    - A thread is created at a priority of 89.
 *    -	It periodically sleeps for a specified duration(period).
 *    - The delay is measured as
 *
 *      delay = (now - start - i*period) converted to microseconds
 *
 *	where,
 *	now = CLOCK_MONOTONIC gettime in ns, start = CLOCK_MONOTONIC gettime
 *	at the start of the test, i = iteration number, period = period chosen
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-May-10: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *      2007-Jul-11: Quantiles added by Josh Triplett <josh@kernel.org>
 *      2007-Jul-12: Latency tracing added by Josh Triplett <josh@kernel.org>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define PRIO 89
//#define PERIOD 17*NS_PER_MS
//#define ITERATIONS 100
#define MIN_ITERATIONS 100
#define DEFAULT_ITERATIONS 10000
#define DEF_PERIOD 5*NS_PER_MS
#define DEF_LOAD_MS 1
#define PASS_US 100
#define HIST_BUCKETS 100
#define OVERHEAD 50000		// allow for 50 us of periodic overhead (context switch, etc.)

nsec_t start;
nsec_t end;
static int ret = 0;
static int iterations = 0;
static unsigned long long latency_threshold = 0;
static nsec_t period = DEF_PERIOD;
static unsigned int load_ms = DEF_LOAD_MS;

stats_container_t dat;
stats_container_t hist;
stats_quantiles_t quantiles;
stats_record_t rec;

void usage(void)
{
	rt_help();
	printf("sched_latency specific options:\n");
	printf("  -dLOAD	periodic load in ms (default 1)\n");
	printf("  -lTHRESHOLD   trace latency, with given threshold in us\n");
	printf("  -tPERIOD      period in ms (default 5)\n");
	printf("  -iITERATIONS  number of iterations (default %d)\n",
	       DEFAULT_ITERATIONS);
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'd':
		load_ms = atoi(v);
		break;
	case 'i':
		iterations = atoi(v);
		break;
	case 'l':
		latency_threshold = strtoull(v, NULL, 0);
		break;
	case 't':
		period = strtoull(v, NULL, 0) * NS_PER_MS;
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *periodic_thread(void *arg)
{
	int i;
	nsec_t delay, avg_delay = 0, start_delay, min_delay = -1ULL, max_delay =
	    0;
	int failures = 0;
	nsec_t next = 0, now = 0, sched_delta = 0, delta = 0, prev =
	    0, iter_start;

	/* wait for the specified start time */
	rt_nanosleep_until(start);

	now = rt_gettime();
	start_delay = (now - start) / NS_PER_US;
	iter_start = next = now;

	debug(DBG_INFO, "ITERATION DELAY(US) MAX_DELAY(US) FAILURES\n");
	debug(DBG_INFO, "--------- --------- ------------- --------\n");

	if (latency_threshold) {
		latency_trace_enable();
		latency_trace_start();
	}
	for (i = 0; i < iterations; i++) {
		/* wait for the period to start */
		next += period;
		prev = now;
		now = rt_gettime();

		if (next < now) {
			printf("\nPERIOD MISSED!\n");
			printf("     scheduled delta: %8llu us\n",
			       sched_delta / 1000);
			printf("	actual delta: %8llu us\n",
			       delta / 1000);
			printf("	     latency: %8llu us\n",
			       (delta - sched_delta) / 1000);
			printf("---------------------------------------\n");
			printf("      previous start: %8llu us\n",
			       (prev - iter_start) / 1000);
			printf("		 now: %8llu us\n",
			       (now - iter_start) / 1000);
			printf("     scheduled start: %8llu us\n",
			       (next - iter_start) / 1000);
			printf("next scheduled start is in the past!\n");
			ret = 1;
			break;
		}

		sched_delta = next - now;	/* how long we should sleep */
		delta = 0;
		do {
			nsec_t new_now;

			rt_nanosleep(next - now);
			new_now = rt_gettime();
			delta += new_now - now;	/* how long we did sleep */
			now = new_now;
		} while (now < next);

		/* start of period */
		delay =
		    (now - iter_start - (nsec_t) (i + 1) * period) / NS_PER_US;
		rec.x = i;
		rec.y = delay;
		stats_container_append(&dat, rec);

		if (delay < min_delay)
			min_delay = delay;
		if (delay > max_delay)
			max_delay = delay;
		if (delay > pass_criteria) {
			failures++;
			ret = 1;
		}
		avg_delay += delay;
		if (latency_threshold && delay > latency_threshold)
			break;

		/* continuous status ticker */
		debug(DBG_INFO, "%9i %9llu %13llu %8i\r", i, delay, max_delay,
		      failures);
		fflush(stdout);

		busy_work_ms(load_ms);
	}
	if (latency_threshold) {
		latency_trace_stop();
		if (i != iterations) {
			printf
			    ("Latency threshold (%lluus) exceeded at iteration %d\n",
			     latency_threshold, i);
			latency_trace_print();
			stats_container_resize(&dat, i + 1);
		}
	}

	/* save samples before the quantile calculation messes things up! */
	stats_hist(&hist, &dat);
	stats_container_save("samples",
			     "Periodic Scheduling Latency Scatter Plot",
			     "Iteration", "Latency (us)", &dat, "points");
	stats_container_save("hist", "Periodic Scheduling Latency Histogram",
			     "Latency (us)", "Samples", &hist, "steps");

	avg_delay /= i;
	printf("\n\n");
	printf("Start: %4llu us: %s\n", start_delay,
	       start_delay < pass_criteria ? "PASS" : "FAIL");
	printf("Min:   %4llu us: %s\n", min_delay,
	       min_delay < pass_criteria ? "PASS" : "FAIL");
	printf("Max:   %4llu us: %s\n", max_delay,
	       max_delay < pass_criteria ? "PASS" : "FAIL");
	printf("Avg:   %4llu us: %s\n", avg_delay,
	       avg_delay < pass_criteria ? "PASS" : "FAIL");
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
	printf("Failed Iterations: %d\n", failures);

	return NULL;
}

int main(int argc, char *argv[])
{
	int per_id;
	setup();

	pass_criteria = PASS_US;
	rt_init("d:l:ht:i:", parse_args, argc, argv);

	printf("-------------------------------\n");
	printf("Scheduling Latency\n");
	printf("-------------------------------\n\n");

	if (load_ms * NS_PER_MS >= period - OVERHEAD) {
		printf("ERROR: load must be < period - %d us\n",
		       OVERHEAD / NS_PER_US);
		exit(1);
	}

	if (iterations == 0)
		iterations = DEFAULT_ITERATIONS;
	if (iterations < MIN_ITERATIONS) {
		printf
		    ("Too few iterations (%d), use min iteration instead (%d)\n",
		     iterations, MIN_ITERATIONS);
		iterations = MIN_ITERATIONS;
	}

	printf("Running %d iterations with a period of %llu ms\n", iterations,
	       period / NS_PER_MS);
	printf("Periodic load duration: %d ms\n", load_ms);
	printf("Expected running time: %d s\n",
	       (int)(iterations * ((float)period / NS_PER_SEC)));

	if (stats_container_init(&dat, iterations))
		exit(1);

	if (stats_container_init(&hist, HIST_BUCKETS)) {
		stats_container_free(&dat);
		exit(1);
	}

	/* use the highest value for the quantiles */
	if (stats_quantiles_init(&quantiles, (int)log10(iterations))) {
		stats_container_free(&hist);
		stats_container_free(&dat);
		exit(1);
	}

	/* wait one quarter second to execute */
	start = rt_gettime() + 250 * NS_PER_MS;
	per_id = create_fifo_thread(periodic_thread, NULL, PRIO);

	join_thread(per_id);
	join_threads();

	printf("\nCriteria: latencies < %d us\n", (int)pass_criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	stats_container_free(&dat);
	stats_container_free(&hist);
	stats_quantiles_free(&quantiles);

	return ret;
}
