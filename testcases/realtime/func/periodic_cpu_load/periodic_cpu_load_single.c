/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006, 2008
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
 *      periodic_cpu_load_single.c
 *
 * DESCRIPTION
 *      Measure variation in computational execution time
 *      at the specified period, priority, and loops.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-May-2: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define HIST_BUCKETS 100

// define sane defaults
#define DEFAULT_ITERATIONS 10000	/* 1000 is the min for 3 nines */
#define DEFAULT_PERIOD 5
#define DEFAULT_PRIO   90
#define DEFAULT_CALC_LOOPS 1000
#define LOOPS_MULTIPLIER 4.2
#define DEFAULT_FILENAME_PREFIX "pcl"

static int prio;
static int period;
static int calc_loops;
static int ret = 0;
static char *filename_prefix = DEFAULT_FILENAME_PREFIX;
static int iterations = DEFAULT_ITERATIONS;

void usage(void)
{
	rt_help();
	printf("periodic_cpu_load_single specific options:\n");
	printf("  -lCALC_LOOPS	loops per iteration\n");
	printf("  -fFILENAME_PREFIX    filename prefix for plot output\n");
	printf
	    ("  -iITERATIONS  number of iterations to calculate the average over\n");
	printf("  -r[0-99]	real-time priority\n");
	printf("  -tPERIOD	period in ms\n");
}

void *calc(int loops)
{
	int i, j;
	for (i = 0; i < loops * LOOPS_MULTIPLIER; i++) {
		for (j = 0; j < 125; j++) {
			// Sum of the numbers up to J
			volatile int temp = j * (j + 1) / 2;
			(void)temp;
		}
	}
	return NULL;
}

int periodic_thread(nsec_t period, int iterations, int loops)
{
	stats_container_t dat;
	stats_container_t hist;
	stats_quantiles_t quantiles;
	stats_record_t rec;

	int i = 0;
	int fail = 0;
	nsec_t next, now;
	nsec_t exe_start, exe_end, exe_time;
	char *samples_filename;
	char *hist_filename;

	stats_container_init(&dat, iterations);
	stats_container_init(&hist, HIST_BUCKETS);
	stats_quantiles_init(&quantiles, (int)log10(iterations));
	if (asprintf(&samples_filename, "%s-samples", filename_prefix) == -1) {
		fprintf(stderr,
			"Failed to allocate string for samples filename\n");
		return -1;
	}

	if (asprintf(&hist_filename, "%s-hist", filename_prefix) == -1) {
		fprintf(stderr,
			"Failed to allocate string for samples filename\n");
		return -1;
	}
	next = rt_gettime();
	while (i < iterations) {
		next += period;
		now = rt_gettime();
		if (now > next) {
			printf
			    ("Missed period, aborting (didn't get scheduled in time)\n");
			fail = 1;
			break;
		}
		exe_start = rt_gettime();
		calc(loops);
		exe_end = rt_gettime();
		exe_time = exe_end - exe_start;
		rec.x = i;
		rec.y = exe_time / NS_PER_US;
		stats_container_append(&dat, rec);

		i++;

		now = rt_gettime();
		if (now > next) {
			printf
			    ("Missed period, aborting (calc took too long)\n");
			fail = 1;
			break;
		}
		rt_nanosleep(next - now);
	}

	stats_container_save(samples_filename, "Periodic CPU Load Scatter Plot",
			     "Iteration", "Runtime (us)", &dat, "points");
	stats_container_save(hist_filename, "Periodic CPU Load Histogram",
			     "Runtime (us)", "Samples", &hist, "steps");

	printf("  Execution Time Statistics:\n");
	printf("Min: %ld us\n", stats_min(&dat));
	printf("Max: %ld us\n", stats_max(&dat));
	printf("Avg: %.4f us\n", stats_avg(&dat));
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
	printf("Criteria: no missed periods\n");
	printf("Result: %s\n", fail ? "FAIL" : "PASS");

	free(samples_filename);
	free(hist_filename);

	return fail;
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'l':
		calc_loops = atoi(v);
		break;
	case 'f':
		filename_prefix = v;
		break;
	case 'h':
		usage();
		exit(0);
	case 'i':
		iterations = atoi(v);
		break;
	case 'r':
		prio = atoi(v);
		break;
	case 't':
		period = atoi(v) * NS_PER_MS;
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

int main(int argc, char *argv[])
{
	period = DEFAULT_PERIOD * NS_PER_MS;
	prio = DEFAULT_PRIO;
	calc_loops = DEFAULT_CALC_LOOPS;
	setup();

	rt_init("f:hi:r:t:l:", parse_args, argc, argv);

	if (iterations < 100) {
		printf("Number of iterations cannot be less than 100\n");
		exit(1);
	}

	if (!period || !prio | !calc_loops) {
		usage();
		exit(1);
	}

	set_priority(prio);

	printf("------------------------------------\n");
	printf("Periodic CPU Load Execution Variance\n");
	printf("------------------------------------\n\n");
	printf("Running %d iterations\n", iterations);
	printf("priority: %d\n", prio);
	printf("  period: %d ms\n", period / NS_PER_MS);
	printf("   loops: %d\n", calc_loops);
	printf("    logs: %s*\n", filename_prefix);

	ret = periodic_thread(period, iterations, calc_loops);

	return ret;
}
