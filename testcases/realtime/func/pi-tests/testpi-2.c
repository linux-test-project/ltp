/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2005, 2008
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
 *      testpi-2.c
 *
 * DESCRIPTION
 *      This testcase verifies if the low priority SCHED_RR thread can preempt
 *      the high priority SCHED_RR thread multiple times via priority
 *      inheritance.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *
 *
 * HISTORY
 *      2010-04-22 Code cleanup and thread synchronization changes by using
 *		 conditional variables,
 *		 by Gowrishankar(gowrishankar.m@in.ibm.com).
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "librttest.h"

pthread_barrier_t barrier;

void usage(void)
{
	rt_help();
	printf("testpi-2 specific options:\n");
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

int gettid(void)
{
	return syscall(__NR_gettid);
}

typedef void *(*entrypoint_t) (void *);
pthread_mutex_t glob_mutex;
static pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void *func_lowrt(void *arg)
{
	struct thread *pthr = (struct thread *)arg;
	int i, tid = gettid();

	printf("Thread %d started running with priority %d\n", tid,
	       pthr->priority);
	pthread_mutex_lock(&glob_mutex);
	printf("Thread %d at start pthread pol %d pri %d - Got global lock\n",
	       tid, pthr->policy, pthr->priority);
	/* Wait for other RT threads to start up */
	pthread_barrier_wait(&barrier);

	/* Wait for the high priority noise thread to start and signal us */
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_wait(&cond_var, &cond_mutex);
	pthread_mutex_unlock(&cond_mutex);

	for (i = 0; i < 10000; i++) {
		if (i % 100 == 0) {
			printf("Thread %d loop %d pthread pol %d pri %d\n",
			       tid, i, pthr->policy, pthr->priority);
			fflush(NULL);
		}
		busy_work_ms(1);
	}
	pthread_mutex_unlock(&glob_mutex);
	return NULL;
}

void *func_rt(void *arg)
{
	struct thread *pthr = (struct thread *)arg;
	int i, tid = gettid();

	printf("Thread %d started running with prio %d\n", tid, pthr->priority);
	pthread_barrier_wait(&barrier);
	pthread_mutex_lock(&glob_mutex);
	printf("Thread %d at start pthread pol %d pri %d - Got global lock\n",
	       tid, pthr->policy, pthr->priority);

	/* We just use the mutex as something to slow things down,
	 * say who we are and then do nothing for a while.  The aim
	 * of this is to show that high priority threads make more
	 * progress than lower priority threads..
	 */
	for (i = 0; i < 1000; i++) {
		if (i % 100 == 0) {
			printf("Thread %d loop %d pthread pol %d pri %d\n",
			       tid, i, pthr->policy, pthr->priority);
			fflush(NULL);
		}
		busy_work_ms(1);
	}
	pthread_mutex_unlock(&glob_mutex);
	return NULL;
}

void *func_noise(void *arg)
{
	struct thread *pthr = (struct thread *)arg;
	int i, tid = gettid();

	printf("Noise Thread %d started running with prio %d\n", tid,
	       pthr->priority);
	pthread_barrier_wait(&barrier);

	/* Let others wait at conditional variable */
	usleep(1000);

	/* Noise thread begins the test */
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_broadcast(&cond_var);
	pthread_mutex_unlock(&cond_mutex);

	for (i = 0; i < 10000; i++) {
		if (i % 100 == 0) {
			printf("Noise Thread %d loop %d pthread pol %d "
			       "pri %d\n", tid, i, pthr->policy,
			       pthr->priority);
			fflush(NULL);
		}
		busy_work_ms(1);
	}
	return NULL;
}

/*
 * Test pthread creation at different thread priorities.
 */
int main(int argc, char *argv[])
{
	int i, retc, nopi = 0;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	setup();
	rt_init("h", parse_args, argc, argv);

	retc = pthread_barrier_init(&barrier, NULL, 5);
	if (retc) {
		printf("pthread_barrier_init failed: %s\n", strerror(retc));
		exit(retc);
	}

	retc = sched_setaffinity(0, sizeof(mask), &mask);
	if (retc < 0) {
		printf("Main Thread: Can't set affinity: %d %s\n", retc,
		       strerror(retc));
		exit(-1);
	}

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "nopi") == 0)
			nopi = 1;
	}

	printf("Start %s\n", argv[0]);

	if (!nopi)
		init_pi_mutex(&glob_mutex);

	create_rr_thread(func_lowrt, NULL, 10);
	create_rr_thread(func_rt, NULL, 20);
	create_fifo_thread(func_rt, NULL, 30);
	create_fifo_thread(func_rt, NULL, 40);
	create_rr_thread(func_noise, NULL, 40);

	printf("Joining threads\n");
	join_threads();
	printf("Done\n");
	printf("Criteria: Low Priority Thread and High Priority Thread "
	       "should prempt each other multiple times\n");

	pthread_mutex_destroy(&glob_mutex);
	pthread_mutex_destroy(&cond_mutex);
	pthread_cond_destroy(&cond_var);

	return 0;
}
