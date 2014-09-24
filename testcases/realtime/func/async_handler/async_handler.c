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
 *     async_handler.c
 *
 * DESCRIPTION
 *     Measure the latency involved in asynchronous event handlers.
 *     Specifically it measures the latency of the pthread_cond_signal
 *     call until the signalled thread is scheduled.
 *
 * USAGE:
 *     Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *     Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *     2006-Oct-20:   Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *	This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>
#include <getopt.h>

#define SIGNAL_PRIO 89
#define HANDLER_PRIO 89
#define DEFAULT_ITERATIONS 1000000	/* about 1 minute @ 2GHz */
#define HIST_BUCKETS 100
#define PASS_US 100

static nsec_t start;
static nsec_t end;
static int iterations = 0;

#define CHILD_START   0
#define CHILD_WAIT    1
#define CHILD_HANDLED 2
#define CHILD_QUIT    3
atomic_t step;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;

static int ret = 0;

void usage(void)
{
	rt_help();
	printf("async_handler specific options:\n");
	printf
	    ("  -iITERATIONS  number of iterations to calculate the average over\n");
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
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *handler_thread(void *arg)
{

	while (atomic_get(&step) != CHILD_QUIT) {
		pthread_mutex_lock(&mutex);
		atomic_set(CHILD_WAIT, &step);
		if (pthread_cond_wait(&cond, &mutex) != 0) {
			perror("pthead_cond_wait");
			break;
		}
		end = rt_gettime();
		atomic_set(CHILD_HANDLED, &step);
		pthread_mutex_unlock(&mutex);
		while (atomic_get(&step) == CHILD_HANDLED)
			usleep(10);
	}
	printf("handler thread exiting\n");
	return 0;
}

void *signal_thread(void *arg)
{

	int i;
	long delta, max, min;
	stats_container_t dat;
	stats_container_t hist;
	stats_record_t rec;

	stats_container_init(&dat, iterations);
	stats_container_init(&hist, HIST_BUCKETS);

	min = max = 0;
	for (i = 0; i < iterations; i++) {
		/* wait for child to wait on cond, then signal the event */
		while (atomic_get(&step) != CHILD_WAIT)
			usleep(10);
		pthread_mutex_lock(&mutex);
		start = rt_gettime();
		if (pthread_cond_signal(&cond) != 0) {
			perror("pthread_cond_signal");
			atomic_set(CHILD_QUIT, &step);
			break;
		}
		pthread_mutex_unlock(&mutex);

		/* wait for the event handler to schedule */
		while (atomic_get(&step) != CHILD_HANDLED)
			usleep(10);
		delta = (long)((end - start) / NS_PER_US);
		if (delta > pass_criteria)
			ret = 1;
		rec.x = i;
		rec.y = delta;
		stats_container_append(&dat, rec);
		if (i == 0)
			min = max = delta;
		else {
			min = MIN(min, delta);
			max = MAX(max, delta);
		}
		atomic_set((i == iterations - 1) ? CHILD_QUIT : CHILD_START,
			   &step);
	}
	printf("recording statistics...\n");
	printf("Min: %ld us\n", min);
	printf("Max: %ld us\n", max);
	printf("Avg: %.4f us\n", stats_avg(&dat));
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	stats_hist(&hist, &dat);
	stats_container_save("samples",
			     "Asynchronous Event Handling Latency Scatter Plot",
			     "Iteration", "Latency (us)", &dat, "points");
	stats_container_save("hist",
			     "Asynchronous Event Handling Latency Histogram",
			     "Latency (us)", "Samples", &hist, "steps");
	printf("signal thread exiting\n");

	return NULL;
}

int main(int argc, char *argv[])
{
	int signal_id, handler_id;
	setup();

	printf("\n-----------------------------------\n");
	printf("Asynchronous Event Handling Latency\n");
	printf("-----------------------------------\n\n");

	pass_criteria = PASS_US;
	rt_init("i:h", parse_args, argc, argv);

	init_pi_mutex(&mutex);

	atomic_set(CHILD_START, &step);

	if (iterations == 0)
		iterations = DEFAULT_ITERATIONS;
	printf("Running %d iterations\n", iterations);

	handler_id =
	    create_fifo_thread(handler_thread, NULL, HANDLER_PRIO);
	signal_id = create_fifo_thread(signal_thread, NULL, SIGNAL_PRIO);

	join_threads();

	printf("\nCriteria: latencies < %d\n", (int)pass_criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return ret;
}
