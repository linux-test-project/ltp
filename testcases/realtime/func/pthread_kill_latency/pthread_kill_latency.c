/******************************************************************************
 *
 *   Copyright  International Business Machines  Corp., 2007
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
 *      pthread_kill_latency.c
 *
 * DESCRIPTION
 *      Measure the latency involved in sending a signal to a thread
 *      using pthread_kill. Two threads are created - the one that receives the
 *      signal (thread1) and the other that sends the signal (thread2). Before
 *      sending the signal, the thread2 waits for thread1 to initialize, notes the
 *      time and sends pthread_kill signal to thread1. thread2, which has defined a
 *      handler for the signal, notes the time it recieves the signal. The maximum
 *      and the minimum latency is reported.
 *
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      pthread_kill_latency [-v{1234}]
 *
 *      Compilation :gcc pthread_kill_latency.c -lpthread -lrt -lm -D_GNU_SOURCE
 *                       -I/usr/include/nptl -o pthread_kill_latency
 *
 * AUTHOR
 *      Sripathi Kodi <sripathik@in.ibm.com>
 *
 * HISTORY
 *      2006-Jun-28:  Initial version by Sripathi Kodi <sripathik@in.ibm.com>
 *	2007-Nov-07:  Added libstats support by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <librttest.h>
#include <libjvmsim.h>
#include <libstats.h>

#define PRIO 89
#define ITERATIONS 10000
#define HIST_BUCKETS 100
#define QUANTILE_NINES 4
#define THRESHOLD 20
#define SIGNALNUMBER SIGUSR1

/* Get the pthread structure corresponding to this thread id */
#define PTHREADOF(tid) get_thread(tid)->pthread

nsec_t begin, end, max, min;
int fail;
float avg;

atomic_t flag;

static int run_jvmsim=0;

void usage(void)
{
	rt_help();
	printf("pthread_kill_latency specific options:\n");
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


#if 0
/* Set up a signal handler */
int rt_setsighandler(int signum, void(*handler)(int))
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
	sigset_t set, oset;

	stats_container_t dat;
	stats_container_t hist;
	stats_quantiles_t quantiles;

	stats_container_init(&dat, ITERATIONS);
	stats_container_init(&hist, HIST_BUCKETS);
	stats_quantiles_init(&quantiles, QUANTILE_NINES);

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

	/* Warm up */
	for (i = 0; i < 5; i++) {
		sigwait(&set, &sig);
		atomic_set(1, &flag);
	}

	max = 0;
	min = -1;
	avg = 0;
	fail = 0;
	debug(DBG_INFO, "\n\n");
	for (i = 0; i < ITERATIONS; i++) {
		sigwait(&set, &sig);
		end = rt_gettime();
		delta = (end - begin)/NS_PER_US;
		dat.records[i].x = i;
		dat.records[i].y = delta;

		if (delta > THRESHOLD) fail++;

		debug(DBG_INFO, "Iteration %d: Took %ld us. Max = %0.2lf us, Min = %0.2lf us\n", i, delta, (double)max/NS_PER_US, (double)min/NS_PER_US);

		fflush(stdout);
		buffer_print();
		atomic_set(1, &flag);
	}

	stats_container_save("samples", "pthread_kill Latency Scatter Plot",
			     "Iteration", "Latency (us)", &dat, "points");
	stats_container_save("hist", "pthread_kill Latency Histogram",
			     "Latency (us)", "Samples", &hist, "steps");
	stats_hist(&hist, &dat);

	printf("\n");
	printf("Min: %lu us\n", stats_min(&dat));
	printf("Max: %lu us\n", stats_max(&dat));
	printf("Avg: %.4f us\n", stats_avg(&dat));
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);
    printf("Failures: %d\n", fail);
	printf("Criteria: Time < %d us\n", THRESHOLD);
	printf("Result: %s", fail ? "FAIL" : "PASS");
	printf("\n\n");


	return NULL;
}

void *signal_sending_thread(void *arg)
{
	int target_thread = (intptr_t)((struct thread *)arg)->arg;
	int i, ret;

	debug(DBG_INFO, "Signal sending thread: target thread id =%d\n",
	      (int)PTHREADOF(target_thread));

	/* Wait for the receiving thread to initialize */
	while(!atomic_get(&flag)) {usleep(100);};
	atomic_set(0, &flag);

	/* Warm up */
	for (i=0; i<5; i++) {

		debug(DBG_DEBUG, "Sending signal (Warm up). Loopcnt = %d\n", i);

		if ((ret = pthread_kill(PTHREADOF(target_thread), SIGNALNUMBER))) {
			printf("pthread_kill returned %d\n", ret);
		}
		/* Wait till the receiving thread processes the signal */
		while(!atomic_get(&flag)) {usleep(100);};
		atomic_set(0, &flag);
	}
	for (i=0; i<ITERATIONS; i++) {

		debug(DBG_DEBUG, "Sending signal. Loopcnt = %d\n", i);

		/* Record the time just before sending the signal */
		begin = rt_gettime();
		if ((ret = pthread_kill(PTHREADOF(target_thread), SIGNALNUMBER))) {
			printf("pthread_kill returned %d\n", ret);
		}
		/* Wait till the receiving thread processes the signal */
		while (!atomic_get(&flag)) {
			usleep(100);
		}
		atomic_set(0, &flag);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int thr_id1, thr_id2;

	atomic_set(0,&flag);
	setup();

	rt_init("jh", parse_args, argc, argv);	/* we need the buffered print system */

	printf("-------------------------------\n");
	printf("pthread_kill Latency\n");
	printf("-------------------------------\n\n");

	printf("Iterations: %d\n", ITERATIONS);

	if (run_jvmsim) {
		printf("jvmsim enabled\n");
		jvmsim_init();  // Start the JVM simulation
	} else {
		printf("jvmsim disabled\n");
	}

	debug(DBG_DEBUG, "Main creating threads\n");
	fflush(stdout);

	thr_id1 = create_fifo_thread(signal_receiving_thread, (void*)0, PRIO);
	thr_id2 = create_fifo_thread(signal_sending_thread, (void*)(intptr_t)thr_id1, PRIO-1);
//	thr_id2 = create_other_thread(signal_sending_thread, (void*)(intptr_t)thr_id1);

	debug(DBG_DEBUG, "Main joining threads debug\n");
	join_thread(thr_id1);
	join_thread(thr_id2);
	buffer_print();

	return fail;
}
