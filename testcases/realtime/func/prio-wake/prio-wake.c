/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006,  2008
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
 *      prio-wake.c
 *
 * DESCRIPTION
 *      Test priority ordered wakeup with pthread_cond_*
 * * Steps:
 *      - Creates a number of worker threads with increasing FIFO priorities
 *	(by default, num worker threads = num cpus)
 *      - Create a master thread
 *      - The time the worker thread starts running is noted. Each of the
 *	  worker threads then waits on the same _condvar_. The time it
 *	  wakes up also noted.
 *      - Once all the threads finish execution, the start and wakeup times
 *	of all the threads is displayed.
 *      - The output must indicate that the thread wakeup happened in a
 *	  priority order.
 *
 * USAGE:
 *
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Apr-26: Initial version by Darren Hart
 *      2006-May-25: Updated to use new librt.h features
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <sys/syscall.h>
#include <librttest.h>
#include <libstats.h>

volatile int running_threads = 0;
static int rt_threads = 0;
static int locked_broadcast = 1;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex;
static volatile nsec_t beginrun;

static int ret = 0;

void usage(void)
{
	rt_help();
	printf("prio-wake specific options:\n");
	printf("  -n#	   #: number of worker threads\n");
	printf("  -l#	   1:lock the mutex before broadcast, 0:don't\n");
	printf("		defaults to 1\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'n':
		rt_threads = atoi(v);
		break;
	case 'l':
		locked_broadcast = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

struct array {
	int *arr;
	int counter;
};
struct array wakeup = { NULL, 0 };

void *master_thread(void *arg)
{
	int rc;
	nsec_t start;

	/* make sure children are started */
	while (running_threads < rt_threads)
		usleep(1000);
	/* give the worker threads a chance to get to sleep in the kernel
	 * in the unlocked broadcast case. */
	usleep(1000);

	start = rt_gettime() - beginrun;

	printf("%08lld us: Master thread about to wake the workers\n",
	       start / NS_PER_US);
	/* start the children threads */
	if (locked_broadcast)
		rc = pthread_mutex_lock(&mutex);
	rc = pthread_cond_broadcast(&cond);
	if (locked_broadcast)
		rc = pthread_mutex_unlock(&mutex);

	return NULL;
}

void *worker_thread(void *arg)
{
	struct sched_param sched_param;
	int policy;
	int rc;
	int mypri;
	int j;
	nsec_t start, wake;
	j = (intptr_t) arg;

	if (pthread_getschedparam(pthread_self(), &policy, &sched_param) != 0) {
		printf
		    ("ERR: Couldn't get pthread info. Priority value wrong\n");
		mypri = -1;
	} else {
		mypri = sched_param.sched_priority;
	}

	start = rt_gettime() - beginrun;
	debug(0, "%08lld us: RealtimeThread-%03d pri %03d started\n",
	      start / NS_PER_US, j, mypri);

	rc = pthread_mutex_lock(&mutex);
	running_threads++;
	rc = pthread_cond_wait(&cond, &mutex);

	wake = rt_gettime() - beginrun;
	running_threads--;
	wakeup.arr[wakeup.counter++] = mypri;
	debug(0, "%08lld us: RealtimeThread-%03d pri %03d awake\n",
	      wake / NS_PER_US, j, mypri);

	rc = pthread_mutex_unlock(&mutex);

	return NULL;
}

int main(int argc, char *argv[])
{
	int threads_per_prio;
	int numcpus;
	int numprios;
	int prio;
	int i;
	setup();

	rt_init("hn:l:", parse_args, argc, argv);

	if (rt_threads == 0) {
		numcpus = sysconf(_SC_NPROCESSORS_ONLN);
		rt_threads = numcpus;
	}
	wakeup.arr = malloc(rt_threads * sizeof(int));
	wakeup.counter = 0;
	printf("\n-----------------------\n");
	printf("Priority Ordered Wakeup\n");
	printf("-----------------------\n");
	printf("Worker Threads: %d\n", rt_threads);
	printf("Calling pthread_cond_broadcast() with mutex: %s\n\n",
	       locked_broadcast ? "LOCKED" : "UNLOCKED");

	beginrun = rt_gettime();

	init_pi_mutex(&mutex);

	/* calculate the number of threads per priority */
	/* we get num numprios -1 for the workers, leaving one for the master */
	numprios = sched_get_priority_max(SCHED_FIFO) -
	    sched_get_priority_min(SCHED_FIFO);

	threads_per_prio = rt_threads / numprios;
	if (rt_threads % numprios)
		threads_per_prio++;

	/* start the worker threads */
	prio = sched_get_priority_min(SCHED_FIFO);
	for (i = rt_threads; i > 0; i--) {
		if ((i != rt_threads && (i % threads_per_prio) == 0))
			prio++;
		create_fifo_thread(worker_thread, (void *)(intptr_t) i, prio);
	}

	/* start the master thread */
	create_fifo_thread(master_thread, (void *)(intptr_t) i, ++prio);

	/* wait for threads to complete */
	join_threads();

	pthread_mutex_destroy(&mutex);

	printf("\nCriteria: Threads should be woken up in priority order\n");

	for (i = 0; i < (wakeup.counter - 1); i++) {
		if (wakeup.arr[i] < wakeup.arr[i + 1]) {
			printf("FAIL: Thread %d woken before %d\n",
			       wakeup.arr[i], wakeup.arr[i + 1]);
			ret++;
		}
	}
	printf("Result: %s\n", ret ? "FAIL" : "PASS");
	return ret;
}
