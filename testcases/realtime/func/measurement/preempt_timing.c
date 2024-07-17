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
 *      preempt_timing.c
 *
 * DESCRIPTION
 *      This program indicated the preemption delays that may be encountered
 *      by realtime apps. The program runs with the scheduling policy of
 *      SCHED_FIFO at a maximum SCHED_FIFO priority. It is bound to a single
 *      processor and its address space is locked as well. It makes successive
 *      calls to the gettimeofday() function(via inlined assembly to read the
 *      TSC).The value returned between two such consecutive calls is reported
 *	as the latency.
 *      The maximum, minimum and average delays are reported for x pairs of such
 *      calls.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *
 *
 * HISTORY
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdint.h>
#include "librttest.h"

#include "tst_tsc.h"

#define ITERATIONS 1000000ULL
#define INTERVALS 10

void usage(void)
{
	rt_help();
	printf("preempt_timing specific options:\n");
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

unsigned long long sample_list[ITERATIONS];
int main(int argc, char *argv[])
{
	unsigned long long i, j, delta, min, max, avg;
	struct sched_param param;
	cpu_set_t mask;
	int err;

#ifdef TSC_UNSUPPORTED
	printf("Error: test cannot be executed on an arch wihout TSC.\n");
	return ENOTSUP;
#endif

	max = avg = 0;
	min = -1;
	setup();

	rt_init("h", parse_args, argc, argv);

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

	/* BIND TO A SINGLE CPU */
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	err = sched_setaffinity(0, sizeof(mask), &mask);
	if (err < 0) {
		printf("Can't set affinity: %d %s\n", err, strerror(err));
		exit(-1);
	}

	for (j = 0; j < INTERVALS; j++) {
		/* Collect samples */
		for (i = 0; i < ITERATIONS; i++)
			rdtscll(sample_list[i]);

		/* Process samples */
		for (i = 0; i < (ITERATIONS - 1); i++) {
			delta = sample_list[i + 1] - sample_list[i];
			if (delta < min)
				min = delta;
			if (delta > max)
				max = delta;
			if (delta > 100000)
				printf("maxd(%llu:%llu): %llu %llu = %llu\n", j,
				       i, sample_list[i], sample_list[i + 1],
				       delta);
			avg += delta;
		}
		usleep(100);	/*let necessary things happen */
	}
	avg /= (ITERATIONS * INTERVALS);

	printf("%lld pairs of gettimeofday() calls completed\n",
	       ITERATIONS * INTERVALS);
	printf("Time between calls:\n");
	printf("Minimum: %llu \n", min);
	printf("Maximum: %llu \n", max);
	printf("Average: %llu \n", avg);

	return 0;
}
