/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that pthread_mutex_unlock()
 * shall wakeup a high priority thread even when a low priority thread
 * is running
 *
 * Steps:
 * 1. Create a mutex and lock
 * 2. Create a high priority thread and make it wait on the mutex
 * 3. Create a low priority thread and let it busy-loop
 * 4. Both low and high prio threads run on same CPU
 * 5. Unlock the mutex and make sure that the higher priority thread
 *    got woken up and preempted low priority thread
 */

#include "affinity.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "posixtest.h"
#include "safe_helpers.h"

#define TEST "5-5"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define HIGH_PRIORITY 10
#define MID_PRIORITY 7
#define LOW_PRIORITY 5
#define RUNTIME 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static volatile int woken_up;
static volatile int low_done;

float timediff(struct timespec t2, struct timespec t1)
{
	float diff = t2.tv_sec - t1.tv_sec;
	diff += (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
	return diff;
}

void *hi_prio_thread(void *tmp)
{
	struct sched_param param;
	int policy;

	(void) tmp;
	set_affinity_single();

	SAFE_PFUNC(pthread_getschedparam(pthread_self(), &policy, &param));
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "The policy is not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != HIGH_PRIORITY) {
		printf(ERROR_PREFIX "The priority is not correct\n");
		exit(PTS_UNRESOLVED);
	}

	SAFE_PFUNC(pthread_mutex_lock(&mutex));

	/* This variable is unprotected because the scheduling removes
	 * the contention
	 */
	if (!low_done)
		woken_up = 1;

	SAFE_PFUNC(pthread_mutex_unlock(&mutex));
	pthread_exit(NULL);
}

void *low_prio_thread(void *tmp)
{
	struct timespec current_time, start_time;
	struct sched_param param;
	int policy;

	(void) tmp;
	set_affinity_single();

	SAFE_PFUNC(pthread_getschedparam(pthread_self(), &policy, &param));
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "Policy not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != LOW_PRIORITY) {
		printf(ERROR_PREFIX "Priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	clock_gettime(CLOCK_REALTIME, &start_time);
	while (!woken_up) {
		clock_gettime(CLOCK_REALTIME, &current_time);
		if (timediff(current_time, start_time) > RUNTIME)
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
	int policy;

	param.sched_priority = MID_PRIORITY;
	SAFE_PFUNC(pthread_setschedparam(pthread_self(), SCHED_RR, &param));
	SAFE_PFUNC(pthread_getschedparam(pthread_self(), &policy, &param));
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "The policy is not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != MID_PRIORITY) {
		printf(ERROR_PREFIX "The priority is not correct\n");
		exit(PTS_UNRESOLVED);
	}

	SAFE_PFUNC(pthread_mutex_lock(&mutex));

	/* create the higher priority */
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

	sleep(1);

	/* Wake the other high priority thread up */
	SAFE_PFUNC(pthread_mutex_unlock(&mutex));

	/* Wait for the threads to exit */
	SAFE_PFUNC(pthread_join(low_id, NULL));
	if (!woken_up) {
		printf("High priority was not woken up. Test FAILED.\n");
		exit(PTS_FAIL);
	}
	SAFE_PFUNC(pthread_join(high_id, NULL));

	printf("Test PASSED\n");
	exit(PTS_PASS);
}
