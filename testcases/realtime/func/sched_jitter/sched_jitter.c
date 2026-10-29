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
 *      sched_jitter.c
 *
 * DESCRIPTION
 *      This test measures scheduling jitter w/ realtime processes.
 *
 *      It spawns a realtime thread that repeatedly times how long it takes to
 *      do a fixed amount of work. It then prints out the maximum jitter seen
 *      (longest execution time - the shortest execution time).
 *      It also spawns off a realtime thread of higher priority that simply
 *      wakes up and goes back to sleep. This tries to measure how much overhead
 *      the scheduler adds in switching quickly to another task and back.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      John Stultz <johnstul@us.ibm.com>
 *
 * HISTORY
 *      2006-May-05: Initial version by John Stultz <johnstul@us.ibm.com>
 *      2007-July-18: Support to gather stats by Ankita Garg <ankita@in.ibm.com>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "libstats.h"
#include "librttest.h"

#define NUMRUNS 1000
#define NUMLOOPS 1000000
#define NSEC_PER_SEC 1000000000
#define WORKLEN 64
#define ISLEEP 50000

int array[WORKLEN];

volatile int flag;		/*let interrupter know we're done */

void usage(void)
{
	rt_help();
	printf("sched_jitter specific options:\n");
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

unsigned long long ts_sub(struct timespec a, struct timespec b)
{
	unsigned long long first, second;

	first = (unsigned long long)a.tv_sec * NSEC_PER_SEC + a.tv_nsec;
	second = (unsigned long long)b.tv_sec * NSEC_PER_SEC + b.tv_nsec;
	return first - second;
}

void print_unit(unsigned long long val)
{
	if (val > 1000000)
		printf("%f ms\n", (float)(val) / 1000000);
	else if (val > 1000)
		printf("%f us\n", (float)(val) / 1000);
	else
		printf("%f ns\n", (float)val);

}

void do_work(int runs)
{
	int i, j;
	for (i = 0; i < runs; i++) {
		for (j = 0; j < WORKLEN - 1; j++)
			array[j] = array[j] + array[j + 1];
		for (j = 0; j < WORKLEN - 1; j++)
			array[j] = array[j] - array[j + 1];
	}
}

void *thread_worker(void *arg)
{
	struct timespec start, stop;
	int i;
	unsigned long long delta;
	unsigned long long min = -1, max = 0;

	stats_container_t dat;
	stats_record_t rec;

	stats_container_init(&dat, NUMRUNS);

	for (i = 0; i < NUMRUNS; i++) {

		do_work(1);	/* warm cache */

		/* do test */
		clock_gettime(CLOCK_MONOTONIC, &start);
		do_work(NUMLOOPS);
		clock_gettime(CLOCK_MONOTONIC, &stop);

		/* calc delta, min and max */
		delta = ts_sub(stop, start);
		if (delta < min)
			min = delta;
		if (delta > max)
			max = delta;
		rec.x = i;
		rec.y = delta;
		stats_container_append(&dat, rec);

		printf("delta: %llu ns\n", delta);
		usleep(1);	/* let other things happen */
	}

	printf("max jitter: ");
	print_unit(max - min);
	stats_container_save("samples", "Scheduling Jitter Scatter Plot",
			     "Iteration", "Delay (ns)", &dat, "points");
	return NULL;
}

void *thread_interrupter(void *arg)
{
	while (!flag)
		usleep(ISLEEP);
	return NULL;
}

int main(int argc, char *argv[])
{
	int worker, interrupter;

	setup();

	rt_init("h", parse_args, argc, argv);

	interrupter = create_fifo_thread(thread_interrupter, NULL, 80);
	sleep(1);
	worker = create_fifo_thread(thread_worker, NULL, 10);

	join_thread(worker);
	flag = 1;
	join_thread(interrupter);

	return 0;
}
