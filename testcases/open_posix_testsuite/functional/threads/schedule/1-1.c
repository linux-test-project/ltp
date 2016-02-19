/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that pthread_barrier_wait()
 * shall wakeup a high priority thread even when a low priority thread
 * is running
 *
 * Steps:
 * 1. Create a barrier object
 * 2. Create a high priority thread and make it wait on the barrier
 * 3. Create a low priority thread and let it busy-loop
 * 4. Setup a signal handler for ALRM
 * 5. Call the final barrier_wait in the signal handler
 * 6. Check that the higher priority thread got woken up
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "posixtest.h"
#include "safe_helpers.h"

#define TEST "5-4"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define HIGH_PRIORITY 10
#define LOW_PRIORITY 5
#define RUNTIME 5

pthread_barrier_t barrier;
static volatile int woken_up;
static volatile int low_done;

float timediff(struct timespec t2, struct timespec t1)
{
	float diff = t2.tv_sec - t1.tv_sec;
	diff += (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
	return diff;
}

int my_pthread_barrier_wait(pthread_barrier_t *p)
{
	int rc;

	rc = pthread_barrier_wait(p);
	if (rc == PTHREAD_BARRIER_SERIAL_THREAD)
		rc = 0;
	return rc;
}

void signal_handler(int sig)
{
	(void) sig;
	SAFE_PFUNC(my_pthread_barrier_wait(&barrier));
}

void *hi_prio_thread(void *tmp)
{
	struct sched_param param;
	int policy;
	void *previous_signal;

	(void) tmp;

	SAFE_PFUNC(pthread_getschedparam(pthread_self(), &policy, &param));
	if (policy != SCHED_RR || param.sched_priority != HIGH_PRIORITY) {
		printf("Error: the policy or priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	previous_signal = signal(SIGALRM, signal_handler);
	if (previous_signal == SIG_ERR) {
		perror(ERROR_PREFIX "signal");
		exit(PTS_UNRESOLVED);
	}

	alarm(2);

	SAFE_PFUNC(my_pthread_barrier_wait(&barrier));

	/* This variable is unprotected because the scheduling removes
	 * the contention
	 */
	if (!low_done)
		woken_up = 1;

	pthread_exit(NULL);
}

void *low_prio_thread(void *tmp)
{
	struct timespec start_timespec, current_timespec;
	struct sched_param param;
	int policy;

	(void) tmp;

	SAFE_PFUNC(pthread_getschedparam(pthread_self(), &policy, &param));
	if (policy != SCHED_RR || param.sched_priority != LOW_PRIORITY) {
		printf("Error: the policy or priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	clock_gettime(CLOCK_REALTIME, &start_timespec);
	while (!woken_up) {
		clock_gettime(CLOCK_REALTIME, &current_timespec);
		if (timediff(current_timespec, start_timespec) > RUNTIME)
			break;
	}
	low_done = 1;

	pthread_exit(NULL);
}

int main()
{
	pthread_t high_id, low_id;
	pthread_attr_t low_attr, high_attr;
	struct sched_param param;

	SAFE_PFUNC(pthread_barrier_init(&barrier, NULL, 2));

	/* Create the higher priority */
	SAFE_PFUNC(pthread_attr_init(&high_attr));
	SAFE_PFUNC(pthread_attr_setinheritsched(&high_attr, PTHREAD_EXPLICIT_SCHED));
	SAFE_PFUNC(pthread_attr_setschedpolicy(&high_attr, SCHED_RR));
	param.sched_priority = HIGH_PRIORITY;
	SAFE_PFUNC(pthread_attr_setschedparam(&high_attr, &param));
	SAFE_PFUNC(pthread_create(&high_id, &high_attr, hi_prio_thread, NULL));

	/* Create the low priority thread */
	SAFE_PFUNC(pthread_attr_init(&low_attr));
	SAFE_PFUNC(pthread_attr_setinheritsched(&low_attr, PTHREAD_EXPLICIT_SCHED));
	SAFE_PFUNC(pthread_attr_setschedpolicy(&low_attr, SCHED_RR));
	param.sched_priority = LOW_PRIORITY;
	SAFE_PFUNC(pthread_attr_setschedparam(&low_attr, &param));
	SAFE_PFUNC(pthread_create(&low_id, &low_attr, low_prio_thread, NULL));

	/* Wait for the threads to exit */
	SAFE_PFUNC(pthread_join(high_id, NULL));
	SAFE_PFUNC(pthread_join(low_id, NULL));

	if (!woken_up) {
		printf("High priority was not woken up. Test FAILED\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	exit(PTS_PASS);
}
