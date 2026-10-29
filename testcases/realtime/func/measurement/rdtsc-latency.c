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
 *      rdtsc-latency.c
 *
 * DESCRIPTION
 *       Simple program to measure the time between several pairs of calls to
 *       rdtsc().  Based off of gtod-latency.c
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      Use "-j" to enable jvm simulator.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Nov-15: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>
#include <stdint.h>
#include "librttest.h"

#include "tst_tsc.h"

#define ITERATIONS 1000000

void usage(void)
{
	rt_help();
	printf("rdtsc-latency specific options:\n");
	printf(" This testcase don't expect any commandline options\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

/* return difference in nanoseconds */
unsigned long long tv_minus(struct timeval *tv_start, struct timeval *tv_end)
{
	unsigned long long nsecs;
	nsecs = (tv_end->tv_sec - tv_start->tv_sec) * 1000000000ULL;
	nsecs += (tv_end->tv_usec - tv_start->tv_usec) * 1000;
	return nsecs;
}

/* calculate the tsc period */
unsigned long long tsc_period_ps(void)
{
	struct timeval tv_start;
	struct timeval tv_end;
	unsigned long long tsc_start, tsc_end;

	rdtscll(tsc_start);
	gettimeofday(&tv_start, NULL);
	sleep(1);
	rdtscll(tsc_end);
	gettimeofday(&tv_end, NULL);

	return (1000 * tv_minus(&tv_start, &tv_end)) / tsc_minus(tsc_start,
								 tsc_end);
}

int main(int argc, char *argv[])
{
	int i, err;
	unsigned long long deltas[ITERATIONS];
	unsigned long long max, min, avg, tsc_a, tsc_b, tsc_period;
	struct sched_param param;

#ifdef TSC_UNSUPPORTED
	printf("Error: test cannot be executed on an arch wihout TSC.\n");
	return ENOTSUP;
#endif

	setup();

	rt_init("h", parse_args, argc, argv);

	/* no arguments */
	if (argc > 1) {
		fprintf(stderr, "%s accepts no arguments\n", argv[0]);
		exit(1);
	}

	/* switch to SCHED_FIFO 99 */
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	err = sched_setscheduler(0, SCHED_FIFO, &param);

	/* Check that the user has the appropriate privileges */
	if (err) {
		if (errno == EPERM) {
			fprintf(stderr,
				"This program runs with a scheduling policy of SCHED_FIFO at priority %d\n",
				param.sched_priority);
			fprintf(stderr,
				"You don't have the necessary privileges to create such a real-time process.\n");
		} else {
			fprintf(stderr, "Failed to set scheduler, errno %d\n",
				errno);
		}
		exit(1);
	}

	/* calculate the tsc period in picoseconds */
	tsc_period = tsc_period_ps();

	/* collect ITERATIONS pairs of gtod calls */
	max = min = avg = 0;
	for (i = 0; i < ITERATIONS; i++) {
		rdtscll(tsc_a);
		rdtscll(tsc_b);
		deltas[i] = (tsc_minus(tsc_a, tsc_b) * tsc_period) / 1000;	/* tsc period is in ps */
		if (i == 0 || deltas[i] < min)
			min = deltas[i];
		if (deltas[i] > max)
			max = deltas[i];
		avg += deltas[i];
	}
	avg /= ITERATIONS;

	/* report on deltas */
	printf("Calculated tsc period = %llu ps\n", tsc_period);
	printf("%d pairs of rdtsc() calls completed\n", ITERATIONS);
	printf("Time between calls:\n");
	printf("     Max: %llu ns\n", max);
	printf("     Min: %llu ns\n", min);
	printf("     Avg: %llu ns\n", avg);

	return 0;
}
