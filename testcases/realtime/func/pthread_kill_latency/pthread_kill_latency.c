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
 *      pthread_kill_latency.c
 *
 * DESCRIPTION
 *      Measure the latency involved in sending a signal to a thread
 *      using pthread_kill. Two threads are created - the one that receives the
 *      signal (thread1) and the other that sends the signal (thread2). Before
 *      sending the signal, the thread2 waits for thread1 to initialize, notes
 *      the time and sends pthread_kill signal to thread1. thread2, which has
 *      defined a handler for the signal, notes the time it receives the signal.
 *      The maximum and the minimum latency is reported.
 *
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      pthread_kill_latency [-v{1234}]
 *
 * AUTHOR
 *      Sripathi Kodi <sripathik@in.ibm.com>
 *
 * HISTORY
 *     	2006-Jun-28:  Initial version by Sripathi Kodi <sripathik@in.ibm.com>
 *	2007-Nov-07:  Added libstats support by Darren Hart <dvhltc@us.ibm.com>
 *	2008-Jan-23:  Latency tracing added by
 *				Sebastien Dugue <sebastien.dugue@bull.net>
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include "librttest.h"
#include "libstats.h"

#define PRIO 89
#define ITERATIONS 10000
#define HIST_BUCKETS 100
#define THRESHOLD 20
#define SIGNALNUMBER SIGUSR1

/* Get the pthread structure corresponding to this thread id */
#define PTHREADOF(tid) get_thread(tid)->pthread

static long latency_threshold = 0;
nsec_t begin, end;
int fail;

atomic_t flag;

void usage(void)
{
	rt_help();
	printf("pthread_kill_latency specific options:\n");
	printf("  -l threshold  trace latency with given threshold in us\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'l':
		latency_threshold = strtoull(v, NULL, 0);
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

#if 0
/* Set up a signal handler */
int rt_setsighandler(int signum, void (*handler) (int))
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	if (sigaction(signum, &sa, NULL) != 0) {
		perror("Sigaction failed:\n");
		return 1;
	}
	return 0;
}
#endif

void *signal_receiving_thread(void *arg)
{
	int i, ret, sig;
	long delta;
	long max, min;
	sigset_t set, oset;

	stats_container_t dat;
	stats_container_t hist;
	stats_quantiles_t quantiles;
	stats_record_t rec;

	stats_container_init(&dat, ITERATIONS);
	stats_container_init(&hist, HIST_BUCKETS);
	stats_quantiles_init(&quantiles, (int)log10(ITERATIONS));

	debug(DBG_DEBUG, "Signal receiving thread running\n");

	if ((sigaddset(&set, SIGNALNUMBER))) {
		perror("sigaddset:");
		exit(1);
	}
	if ((ret = pthread_sigmask(SIG_BLOCK, &set, &oset))) {
		printf("pthread_sigmask returned %d\n", ret);
		exit(1);
	}

	/* Let the sending thread know that receiver is ready */
	atomic_set(1, &flag);

	debug(DBG_DEBUG, "Signal receiving thread ready to receive\n");

	if (latency_threshold) {
		latency_trace_enable();
		latency_trace_start();
	}

	/* Warm up */
	for (i = 0; i < 5; i++) {
		sigwait(&set, &sig);
		atomic_set(1, &flag);
	}

	max = min = 0;
	fail = 0;
	debug(DBG_INFO, "\n\n");

	for (i = 0; i < ITERATIONS; i++) {
		sigwait(&set, &sig);
		end = rt_gettime();
		delta = (end - begin) / NS_PER_US;
		rec.x = i;
		rec.y = delta;
		stats_container_append(&dat, rec);

		if (i == 0 || delta < min)
			min = delta;

		if (delta > max)
			max = delta;

		if (delta > pass_criteria)
			fail++;

		debug(DBG_INFO, "Iteration %d: Took %ld us. Max = %ld us, "
		      "Min = %ld us\n", i, delta, max, min);

		fflush(stdout);
		buffer_print();

		if (latency_threshold && (delta > latency_threshold)) {
			atomic_set(2, &flag);
			break;
		}

		atomic_set(1, &flag);
	}

	if (latency_threshold) {
		latency_trace_stop();

		if (i != ITERATIONS) {
			printf
			    ("Latency threshold (%luus) exceeded at iteration %d\n",
			     latency_threshold, i);
			fflush(stdout);
			buffer_print();
			latency_trace_print();
			stats_container_resize(&dat, i + 1);
		}
	}

	stats_hist(&hist, &dat);
	stats_container_save("samples", "pthread_kill Latency Scatter Plot",
			     "Iteration", "Latency (us)", &dat, "points");
	stats_container_save("hist", "pthread_kill Latency Histogram",
			     "Latency (us)", "Samples", &hist, "steps");

	printf("\n");
	printf("Min: %lu us\n", stats_min(&dat));
	printf("Max: %lu us\n", stats_max(&dat));
	printf("Avg: %.4f us\n", stats_avg(&dat));
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
	printf("Failures: %d\n", fail);
	printf("Criteria: Time < %d us\n", (int)pass_criteria);
	printf("Result: %s", fail ? "FAIL" : "PASS");
	printf("\n\n");

	return NULL;
}

void *signal_sending_thread(void *arg)
{
	int target_thread = (intptr_t) ((struct thread *)arg)->arg;
	int i, ret;

	debug(DBG_INFO, "Signal sending thread: target thread id =%d\n",
	      (int)PTHREADOF(target_thread));

	/* Wait for the receiving thread to initialize */
	while (!atomic_get(&flag)) {
		usleep(100);
	};
	atomic_set(0, &flag);

	/* Warm up */
	for (i = 0; i < 5; i++) {

		debug(DBG_DEBUG, "Sending signal (Warm up). Loopcnt = %d\n", i);

		if ((ret =
		     pthread_kill(PTHREADOF(target_thread), SIGNALNUMBER))) {
			printf("pthread_kill returned %d\n", ret);
		}
		/* Wait till the receiving thread processes the signal */
		while (!atomic_get(&flag)) {
			usleep(100);
		};
		atomic_set(0, &flag);
	}
	for (i = 0; i < ITERATIONS; i++) {

		debug(DBG_DEBUG, "Sending signal. Loopcnt = %d\n", i);

		/* Record the time just before sending the signal */
		begin = rt_gettime();
		if ((ret =
		     pthread_kill(PTHREADOF(target_thread), SIGNALNUMBER))) {
			printf("pthread_kill returned %d\n", ret);
		}
		/* Wait till the receiving thread processes the signal */
		while (!atomic_get(&flag)) {
			usleep(100);
		}

		if (atomic_get(&flag) == 2)
			break;

		atomic_set(0, &flag);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int thr_id1, thr_id2;

	atomic_set(0, &flag);
	setup();

	pass_criteria = THRESHOLD;
	rt_init("l:h", parse_args, argc, argv);	/* we need the buffered print system */

	printf("-------------------------------\n");
	printf("pthread_kill Latency\n");
	printf("-------------------------------\n\n");

	printf("Iterations: %d\n", ITERATIONS);

	debug(DBG_DEBUG, "Main creating threads\n");
	fflush(stdout);

	thr_id1 = create_fifo_thread(signal_receiving_thread, NULL, PRIO);
	thr_id2 =
	    create_fifo_thread(signal_sending_thread,
			       (void *)(intptr_t) thr_id1, PRIO - 1);
//      thr_id2 = create_other_thread(signal_sending_thread, (void*)(intptr_t)thr_id1);

	debug(DBG_DEBUG, "Main joining threads debug\n");
	join_thread(thr_id1);
	join_thread(thr_id2);
	buffer_print();

	return fail;
}
