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
 *      periodic_cpu_load.c
 *
 * DESCRIPTION
 *      Measure variation in computational execution time
 *      at various periods and priorities.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-April-27:    Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define PRIO_A 63
#define PRIO_B 53
#define PRIO_C 43

#define PERIOD_A 40*NS_PER_MS
#define PERIOD_B 80*NS_PER_MS
#define PERIOD_C 160*NS_PER_MS

#define CALC_LOOPS_A (27*100)
#define CALC_LOOPS_B (50*100)
#define CALC_LOOPS_C (72*100)

#define NUM_GROUPS 3
#define THREADS_PER_GROUP 4

//#define ITERATIONS 100 /* short functional test run */
#define ITERATIONS 6000		/* about 15 minutes @ 2GHz on 1 CPU */
//#define ITERATIONS 1000 /* min iters for 3 nines */
// FIXME: need some kind of passing criteria calculation
//#define PASS_US 100

int fail[THREADS_PER_GROUP * NUM_GROUPS];
stats_container_t dat[THREADS_PER_GROUP * NUM_GROUPS];
stats_record_t rec;
stats_quantiles_t quantiles[THREADS_PER_GROUP * NUM_GROUPS];
static const char groupname[NUM_GROUPS] = "ABC";

static int iterations = ITERATIONS;
static int ret = 0;

void usage(void)
{
	rt_help();
	printf("periodic_cpu_load specific options:\n");
	printf
	    ("  -iITERATIONS  number of iterations to calculate the average over\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
		break;
	case 'i':
		iterations = atoi(v);
		break;
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

struct periodic_arg {
	int period;
	int iterations;
	void *(*func) (void *);
	void *arg;
};

void *calc(void *arg)
{
	int i, j;
	int loops = (intptr_t) arg;
	for (i = 0; i < loops; i++) {
		for (j = 0; j < 125; j++) {
			// Sum of the numbers up to J
			int temp = j * (j + 1) / 2;
			(void)temp;
		}
	}
	return NULL;
}

void *periodic_thread(void *thread)
{
	struct thread *t = (struct thread *)thread;
	struct periodic_arg *parg = (struct periodic_arg *)t->arg;
	nsec_t period = parg->period;
	void *(*func) (void *) = parg->func;

	int i = 0;
	nsec_t next, now;
	nsec_t exe_start, exe_end, exe_time;

	next = rt_gettime();
	while (i < parg->iterations) {
		next += period;
		if (rt_gettime() > next) {
			printf("TID %d missed period, aborting\n", t->id);
			fail[t->id] = 1;
			break;
		}
		exe_start = rt_gettime();
		func(parg->arg);
		exe_end = rt_gettime();
		exe_time = exe_end - exe_start;
		rec.x = i;
		rec.y = exe_time / NS_PER_US;
		stats_container_append(&dat[t->id], rec);

		i++;

		now = rt_gettime();
		if (now > next) {
			printf
			    ("Missed period, aborting (calc took too long)\n");
			fail[t->id] = 1;
			break;
		}
		rt_nanosleep(next - now);
	}

	printf("TID %d (%c - prio %d) complete\n", t->id, groupname[t->id >> 2],
	       t->priority);

	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	setup();

	rt_init("hi:", parse_args, argc, argv);

	if (iterations < 100) {
		fprintf(stderr,
			"Number of iteration cannot be less than 100.\n");
		exit(1);
	}

	printf("------------------------------------\n");
	printf("Periodic CPU Load Execution Variance\n");
	printf("------------------------------------\n\n");
	printf("Running %d iterations per thread\n", iterations);
	printf("Thread Group A:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_A);
	printf("  period: %d ms\n", PERIOD_A / NS_PER_MS);
	printf("Thread Group B:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_B);
	printf("  period: %d ms\n", PERIOD_B / NS_PER_MS);
	printf("Thread Group C:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_C);
	printf("  period: %d ms\n", PERIOD_C / NS_PER_MS);
	printf("\n");

	for (i = 0; i < (THREADS_PER_GROUP * NUM_GROUPS); i++) {
		stats_container_init(&dat[i], iterations);
		stats_quantiles_init(&quantiles[i], (int)log10(iterations));
	}

	struct periodic_arg parg_a =
	    { PERIOD_A, iterations, calc, (void *)CALC_LOOPS_A };
	struct periodic_arg parg_b =
	    { PERIOD_B, iterations, calc, (void *)CALC_LOOPS_B };
	struct periodic_arg parg_c =
	    { PERIOD_C, iterations, calc, (void *)CALC_LOOPS_C };

	for (i = 0; i < THREADS_PER_GROUP; i++)
		create_fifo_thread(periodic_thread, (void *)&parg_a, PRIO_A);
	for (i = 0; i < THREADS_PER_GROUP; i++)
		create_fifo_thread(periodic_thread, (void *)&parg_b, PRIO_B);
	for (i = 0; i < THREADS_PER_GROUP; i++)
		create_fifo_thread(periodic_thread, (void *)&parg_c, PRIO_C);

	join_threads();

	printf("\nExecution Time Statistics:\n\n");

	for (i = 0; i < (THREADS_PER_GROUP * NUM_GROUPS); i++) {
		printf("TID %d (%c)\n", i, groupname[i >> 2]);
		printf("  Min: %ld us\n", stats_min(&dat[i]));
		printf("  Max: %ld us\n", stats_max(&dat[i]));
		printf("  Avg: %f us\n", stats_avg(&dat[i]));
		printf("  StdDev: %f us\n\n", stats_stddev(&dat[i]));
		printf("  Quantiles:\n");
		stats_quantiles_calc(&dat[i], &quantiles[i]);
		stats_quantiles_print(&quantiles[i]);
		printf("Criteria: TID %d did not miss a period\n", i);
		printf("Result: %s\n", fail[i] ? "FAIL" : "PASS");
		printf("\n");

		if (fail[i])
			ret = 1;
	}

	// FIXME: define pass criteria
	// printf("\nCriteria: latencies < %d us\n", PASS_US);
	// printf("Result: %s\n", ret ? "FAIL" : "PASS");

	for (i = 0; i < (THREADS_PER_GROUP * NUM_GROUPS); i++) {
		stats_container_free(&dat[i]);
		stats_quantiles_free(&quantiles[i]);
	}

	return ret;
}
