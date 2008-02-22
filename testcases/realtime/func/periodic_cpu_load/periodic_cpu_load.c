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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *      Use "-j" to enable jvm simulator.
 *
 *      Compilation : gcc -lrt -lpthread periodic_cpu_load.c
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-April-27:    Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>
#include <libjvmsim.h>

#define PRIO_A 63
#define PRIO_B 53
#define PRIO_C 43

#define PERIOD_A 40*NS_PER_MS
#define PERIOD_B 80*NS_PER_MS
#define PERIOD_C 160*NS_PER_MS

#define CALC_LOOPS_A (27*100)
#define CALC_LOOPS_B (50*100)
#define CALC_LOOPS_C (72*100)

#define THREADS_PER_GROUP 4

//#define ITERATIONS 100 /* short functional test run */
#define ITERATIONS 6000 /* about 15 minutes @ 2GHz on 1 CPU */
//#define ITERATIONS 1000 /* min iters for 3 nines */
// FIXME: need some kind of passing criteria calculation
//#define PASS_US 100

static int run_jvmsim = 0;
static int ret = 0;

void usage(void)
{
        rt_help();
        printf("periodic_cpu_load specific options:\n");
        printf("  -j            enable jvmsim\n");
}

int parse_args(int c, char *v)
{
        int handled = 1;
        switch (c) {
                case 'j':
                        run_jvmsim = 1;
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
	void*(*func)(void*);
	void *arg;
};

void *calc(void *arg)
{
	int i, j;
	int loops = (intptr_t)arg;
	for (i = 0; i < loops; i++) {
		for (j = 0; j < 125; j++ ) {
			// Sum of the numbers up to J
			int temp = j * ( j + 1 ) / 2;
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
	void*(*func)(void*) = parg->func;

	stats_container_t dat;
	stats_quantiles_t quantiles;

	int i = 0;
	int fail = 0;
	nsec_t next, now;
	nsec_t exe_start, exe_end, exe_time;

	stats_container_init(&dat, ITERATIONS);
	stats_quantiles_init(&quantiles, 3);

	next = rt_gettime();
	while (i < parg->iterations) {
		next += period;
		if (rt_gettime() > next) {
			printf("TID %d missed period, aborting\n", t->id);
			fail = 1;
			break;
		}
		exe_start = rt_gettime();
		func(parg->arg);
		exe_end = rt_gettime();
		exe_time = exe_end - exe_start;
		dat.records[i].x = i;
		dat.records[i].y = exe_time/NS_PER_US;

		i++;

		now = rt_gettime();
		if (now > next) {
			printf("Missed period, aborting (calc took too long)\n");
			fail = 1;
			break;
		}
		rt_nanosleep(next - now);
	}

	printf("\nTID %d (prio %d) complete\n", t->id, t->priority);
	printf("  Execution Time Statistics:\n");
	printf("        Min: %ld us\n", stats_min(&dat));
	printf("        Max: %ld us\n", stats_max(&dat));
	printf("        Avg: %f us\n", stats_avg(&dat));
	printf("     StdDev: %f us\n\n", stats_stddev(&dat));
	printf("  Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
	printf("Criteria:TID %d missed a period\n",t->id);
	printf("Result: %s\n", fail ? "FAIL":"PASS");

	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	setup();

	rt_init("jh", parse_args, argc, argv);

	printf("------------------------------------\n");
	printf("Periodic CPU Load Execution Variance\n");
	printf("------------------------------------\n\n");
	printf("Running %d iterations per thread\n", ITERATIONS);
	printf("Thread Group A:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_A);
	printf("  period: %d ms\n", PERIOD_A/NS_PER_MS);
	printf("Thread Group B:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_B);
	printf("  period: %d ms\n", PERIOD_B/NS_PER_MS);
	printf("Thread Group C:\n");
	printf("  threads: %d\n", THREADS_PER_GROUP);
	printf("  priority: %d\n", PRIO_C);
	printf("  period: %d ms\n", PERIOD_C/NS_PER_MS);

	if (run_jvmsim) {
		printf("jvmsim enabled\n");
		jvmsim_init();	// Start the JVM simulation
	} else {
		printf("jvmsim disabled\n");
	}

	struct periodic_arg parg_a = { PERIOD_A, ITERATIONS, calc, (void *)CALC_LOOPS_A };
	struct periodic_arg parg_b = { PERIOD_B, ITERATIONS, calc, (void *)CALC_LOOPS_B };
	struct periodic_arg parg_c = { PERIOD_C, ITERATIONS, calc, (void *)CALC_LOOPS_C };
	for (i=0; i < THREADS_PER_GROUP; i++) {
		create_fifo_thread(periodic_thread, (void*)&parg_a, PRIO_A);
		create_fifo_thread(periodic_thread, (void*)&parg_b, PRIO_B);
		create_fifo_thread(periodic_thread, (void*)&parg_c, PRIO_C);
	}

	join_threads();

	// FIXME: define pass criteria
	// printf("\nCriteria: latencies < %d us\n", PASS_US);
	// printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return ret;
}
