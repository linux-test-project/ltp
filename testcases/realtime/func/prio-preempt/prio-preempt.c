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
 *     prio-preempt.c
 *
 * DESCRIPTION
 *     Test whether priority pre-emption works fine.
 *
 *    The main thread:
 *     - Creates a minimum of (N-1) busy threads at priority starting at
 *		     SCHED_FIFO + 80
 *     - Creates 26 FIFO (T1, T2,...,T26) threads with priorities 10, 11,...,36.
 *     - Each of these worker threads executes the following piece of code:
 *		   pthread_mutex_lock(Mi);
 *		   pthread_cond_wait(CVi);
 *		   pthread_mutex_unlock(Mi);
 *
 *       where Mi is the ith pthread_mutex_t and CVi is the ith conditional
 *       variable.So, at the end of this loop, 26 threads are all waiting on
 *       seperate condvars and mutexes.
 *     - Wakes up thread at priority 10 (T1) by executing:
 *	   pthread_mutex_lock(M1);
 *	   pthread_cond_signal(CV1);
 *	   pthread_mutex_unlock(M1);
 *
 *     - Waits for all the worker threads to finish execution.
 *	 T1 then wakes up T2 by signalling on the condvar CV2 and sets a flag
 *	 called T1_after_wait to indicate that it is after the wait. It then
 *	 checks if T2_after_wait has been set or not. If not, the test fails,
 *	 else the process continues with other threads. The thread T1 expects
 *	 T2_after_wait to be set as, the moment T1 signals on CV2, T2 is
 *	 supposed to be scheduled (in accordance with priority preemption).
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Dinakar Guniguntala <dino@us.ibm.com>
 *
 * HISTORY
 *      2006-Jun-01: Initial version by Dinakar Guniguntala
 *		    Changes from John Stultz and Vivek Pallantla
 *
 *****************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <sys/syscall.h>
#include "librttest.h"

#define NUM_WORKERS	27
#define CHECK_LIMIT	1

volatile int busy_threads = 0;
volatile int test_over = 0;
volatile int threads_running = 0;
static int rt_threads = -1;
static int int_threads = 0;
static pthread_mutex_t bmutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t mutex[NUM_WORKERS + 1];
static pthread_cond_t cond[NUM_WORKERS + 1];
static int t_after_wait[NUM_WORKERS];

static int ret = 0;

pthread_barrier_t barrier;

void usage(void)
{
	rt_help();
	printf("prio-preempt specific options:\n");
	printf("  -i	    #: enable interrupter threads\n");
	printf("  -n#	   #: number of busy threads\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'i':
		int_threads = 1;
		break;
	case 'n':
		rt_threads = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *int_thread(void *arg)
{
	intptr_t a = 0;
	while (!test_over) {
		/* do some busy work */
		if (!(a % 4))
			a = a * 3;
		else if (!(a % 6))
			a = a / 2;
		else
			a++;
		usleep(20);
	}
	return (void *)a;
}

void *busy_thread(void *arg)
{
	struct sched_param sched_param;
	int policy, mypri = 0, tid;
	tid = (intptr_t) (((struct thread *)arg)->arg);

	if (pthread_getschedparam(pthread_self(), &policy, &sched_param) != 0) {
		printf("ERR: Couldn't get pthread info \n");
	} else {
		mypri = sched_param.sched_priority;
	}

	pthread_mutex_lock(&bmutex);
	busy_threads++;
	printf("Busy Thread %d(%d): Running...\n", tid, mypri);
	pthread_mutex_unlock(&bmutex);

	/* TODO: Add sched set affinity here */

	/* Busy loop */
	while (!test_over) ;

	printf("Busy Thread %d(%d): Exiting\n", tid, mypri);
	return NULL;
}

void *worker_thread(void *arg)
{
	struct sched_param sched_param;
	int policy, rc, mypri = 0, tid, times = 0;
	tid = (intptr_t) (((struct thread *)arg)->arg);
	nsec_t pstart, pend;

	if (pthread_getschedparam(pthread_self(), &policy, &sched_param) != 0) {
		printf("ERR: Couldn't get pthread info \n");
	} else {
		mypri = sched_param.sched_priority;
	}
	/* check in */
	pthread_mutex_lock(&bmutex);
	threads_running++;
	pthread_mutex_unlock(&bmutex);

	/* block */
	rc = pthread_mutex_lock(&mutex[tid]);
	if (tid == 0)
		pthread_barrier_wait(&barrier);
	rc = pthread_cond_wait(&cond[tid], &mutex[tid]);
	rc = pthread_mutex_unlock(&mutex[tid]);

	debug(DBG_INFO, "%llu: Thread %d(%d) wakes up from sleep \n",
	      rt_gettime(), tid, mypri);

	/*check if we're the last thread */
	if (tid == NUM_WORKERS - 1) {
		t_after_wait[tid] = 1;
		pthread_mutex_lock(&bmutex);
		threads_running--;
		pthread_mutex_unlock(&bmutex);
		return NULL;
	}

	/* Signal next thread */
	rc = pthread_mutex_lock(&mutex[tid + 1]);
	rc = pthread_cond_signal(&cond[tid + 1]);
	debug(DBG_INFO, "%llu: Thread %d(%d): Sent signal (%d) to (%d)\n",
	      rt_gettime(), tid, mypri, rc, tid + 1);

	pstart = pend = rt_gettime();
	rc = pthread_mutex_unlock(&mutex[tid + 1]);

	debug(DBG_INFO, "%llu: Thread %d(%d) setting it's bit \n", rt_gettime(),
	      tid, mypri);

	t_after_wait[tid] = 1;

	while (t_after_wait[tid + 1] != 1) {
		pend = rt_gettime();
		times++;
	}

	if (times >= (int)pass_criteria) {
		printf
		    ("Thread %d(%d): Non-Preempt limit reached. %llu ns latency\n",
		     tid, mypri, pend - pstart);
		ret = 1;
	}

	/* check out */
	pthread_mutex_lock(&bmutex);
	threads_running--;
	pthread_mutex_unlock(&bmutex);

	return NULL;
}

void *master_thread(void *arg)
{
	int i, pri_boost;

	pthread_barrier_init(&barrier, NULL, 2);

	/* start interrupter thread */
	if (int_threads) {
		pri_boost = 90;
		for (i = 0; i < rt_threads; i++) {
			create_fifo_thread(int_thread, NULL,
					   sched_get_priority_min(SCHED_FIFO) +
					   pri_boost);
		}
	}

	/* start the (N-1) busy threads */
	pri_boost = 80;
	for (i = rt_threads; i > 1; i--) {
		create_fifo_thread(busy_thread, (void *)(intptr_t) i,
				   sched_get_priority_min(SCHED_FIFO) +
				   pri_boost);
	}

	/* make sure children are started */
	while (busy_threads < (rt_threads - 1))
		usleep(100);

	printf("Busy threads created!\n");

	/* start NUM_WORKERS worker threads */
	for (i = 0, pri_boost = 10; i < NUM_WORKERS; i++, pri_boost += 2) {
		pthread_mutex_init(&mutex[i], NULL);
		pthread_cond_init(&cond[i], NULL);
		create_fifo_thread(worker_thread, (void *)(intptr_t) i,
				   sched_get_priority_min(SCHED_FIFO) +
				   pri_boost);
	}

	printf("Worker threads created\n");
	/* Let the worker threads wait on the cond vars */
	while (threads_running < NUM_WORKERS)
		usleep(100);

	/* Ensure the first worker has called cond_wait */
	pthread_barrier_wait(&barrier);

	printf("Signaling first thread\n");
	pthread_mutex_lock(&mutex[0]);
	pthread_cond_signal(&cond[0]);
	pthread_mutex_unlock(&mutex[0]);

	while (threads_running)
		usleep(500000);	/* this period greatly affects the number of failures! */

	test_over = 1;
	return NULL;
}

int get_numcpus(void)
{
	long numcpus_conf = sysconf(_SC_NPROCESSORS_CONF);
	size_t size = CPU_ALLOC_SIZE(numcpus_conf);
	cpu_set_t *cpuset = CPU_ALLOC(numcpus_conf);

	CPU_ZERO_S(size, cpuset);
	/* Get the number of cpus accessible to the current process */
	sched_getaffinity(0, size, cpuset);

	return CPU_COUNT_S(size, cpuset);
}

int main(int argc, char *argv[])
{
	int pri_boost, numcpus;
	setup();

	pass_criteria = CHECK_LIMIT;
	rt_init("hin:", parse_args, argc, argv);

	numcpus = get_numcpus();

	/* Max no. of busy threads should always be less than/equal the no. of
	   housekeeping cpus. Otherwise, the box will hang */

	if (rt_threads == -1 || rt_threads > numcpus) {
		rt_threads = numcpus;
		printf("Maximum busy thread count(%d), "
		       "should not exceed number of cpus(%d)\n", rt_threads,
		       numcpus);
		printf("Using %d\n", numcpus);
	}

	/* Test boilder plate: title and parameters */
	printf("\n-------------------\n");
	printf("Priority Preemption\n");
	printf("-------------------\n\n");
	printf("Busy Threads: %d\n", rt_threads);
	printf("Interrupter Threads: %s\n",
	       int_threads ? "Enabled" : "Disabled");
	printf("Worker Threads: %d\n\n", NUM_WORKERS);

	pri_boost = 81;
	create_fifo_thread(master_thread, NULL,
			   sched_get_priority_min(SCHED_FIFO) + pri_boost);

	/* wait for threads to complete */
	join_threads();

	printf
	    ("\nCriteria: All threads appropriately preempted within %d loop(s)\n",
	     (int)pass_criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");
	return ret;
}
