/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_broadcast()
 *   shall wakeup a high priority thread even when a low priority thread
 *   is running
 *
 * Steps:
 * 1. Create a condition variable
 * 2. Create a high priority thread and make it wait on the cond
 * 3. Create a low priority thread and let it busy-loop
 * 4. Broadcast the cond in a signal handler and check that high
 *    priority thread got woken up

 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "posixtest.h"

#define TEST "5-1"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define HIGH_PRIORITY 10
#define LOW_PRIORITY  5
#define RUNTIME       5
#define POLICY        SCHED_RR

/* mutex required by the cond variable */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/* condition variable that threads block on*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* Flags that the threads use to indicate events */
int woken_up = -1;
int low_done = -1;

/* Signal handler that handle the ALRM and wakes up
 * the high priority thread
 */
void signal_handler(int sig)
{
	if (pthread_cond_broadcast(&cond) != 0) {
		printf(ERROR_PREFIX "pthread_cond_broadcast\n");
		exit(PTS_UNRESOLVED);
	}
}

/* Utility function to find difference between two time values */
float timediff(struct timespec t2, struct timespec t1)
{
	float diff = t2.tv_sec - t1.tv_sec;
	diff += (t2.tv_nsec - t1.tv_nsec)/1000000000.0;
	return diff;
}

void *hi_priority_thread(void *tmp)
{
	struct sched_param        param;
	int                       policy;
	int                       rc = 0;

	param.sched_priority = HIGH_PRIORITY;

	rc = pthread_setschedparam(pthread_self(), POLICY, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	if ((policy != POLICY) || (param.sched_priority != HIGH_PRIORITY)) {
		printf("Error: the policy or priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	/* Install a signal handler for ALRM */
	if (signal(SIGALRM, signal_handler) != 0) {
		perror(ERROR_PREFIX "signal:");
		exit(PTS_UNRESOLVED);
	}

	/* acquire the mutex */
	rc = pthread_mutex_lock(&mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* Setup an alarm to go off in 2 seconds */
	alarm(2);

	/* Block, to be woken up by the signal handler */
	rc = pthread_cond_wait(&cond, &mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_cond_wait\n");
		exit(PTS_UNRESOLVED);
	}

	/* This variable is unprotected because the scheduling removes
	 * the contention
	 */
	if (low_done != 1)
		woken_up = 1;

	rc = pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_unlock\n");
		exit(PTS_UNRESOLVED);
	}
	return NULL;
}

void *low_priority_thread(void *tmp)
{
	struct timespec        start_time, current_time;
	struct sched_param     param;
	int                    policy;
	int                    rc = 0;

	param.sched_priority = LOW_PRIORITY;

	rc = pthread_setschedparam(pthread_self(), POLICY, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	if ((policy != POLICY) || (param.sched_priority != LOW_PRIORITY)) {
		printf("Error: the policy or priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	/* grab the start time and busy loop for 5 seconds */
	clock_gettime(CLOCK_REALTIME, &start_time);
	while (1)
	{
		clock_gettime(CLOCK_REALTIME, &current_time);
		if (timediff(current_time, start_time) > RUNTIME)
			break;
	}
	low_done = 1;
	return NULL;
}

int main()
{
	pthread_t                   high_id, low_id;
	pthread_attr_t              high_attr, low_attr;
	struct sched_param          param;
	int                         rc = 0;

	/* Create the higher priority thread */
	rc = pthread_attr_init(&high_attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_setschedpolicy(&high_attr, POLICY);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
	param.sched_priority = HIGH_PRIORITY;
	rc = pthread_attr_setschedparam(&high_attr, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_create(&high_id, &high_attr, hi_priority_thread, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		exit(PTS_UNRESOLVED);
	}

	/* Create the low priority thread */
	rc = pthread_attr_init(&low_attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_attr_setschedpolicy(&low_attr, POLICY);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
	param.sched_priority = LOW_PRIORITY;
	rc = pthread_attr_setschedparam(&low_attr, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_create(&low_id, &low_attr, low_priority_thread, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		exit(PTS_UNRESOLVED);
	}

	/* Wait for the threads to exit */
	rc = pthread_join(high_id, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_join(low_id, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		exit(PTS_UNRESOLVED);
	}

	/* Check the result */
	if (woken_up == -1) {
		printf("Test FAILED: high priority was not woken up\\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	exit(PTS_PASS);
}