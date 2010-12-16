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
 * 4. Unlock the mutex and make sure that the higher priority thread
 *    got woken up
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "posixtest.h"

#define TEST "5-5"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define HIGH_PRIORITY 10
#define MID_PRIORITY 7
#define LOW_PRIORITY 5
#define RUNTIME 5

/* mutex  high priority thread will wait on*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/* mutex required by the cond variable */
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
/* condition variable that threads block on*/
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

volatile int woken_up = -1;
volatile int low_done = -1;

/* Utility function to find difference between two time values */
float timediff(struct timespec t2, struct timespec t1)
{
	float diff = t2.tv_sec - t1.tv_sec;
	diff += (t2.tv_nsec - t1.tv_nsec)/1000000000.0;
	return diff;
}

/* This signal handler will wakeup the main thread by sending a signal
 * to a condition variable that the main thread is waiting on.
 */
void signal_handler(int sig)
{
	int	 rc = 0;

	rc = pthread_cond_signal(&cond);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_cond_signal\n");
		exit(PTS_UNRESOLVED);
	}
}

void *hi_priority_thread(void *tmp)
{
	struct sched_param        param;
	int                       policy;
	int                       rc = 0;

	param.sched_priority = HIGH_PRIORITY;
	rc = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "The policy is not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != HIGH_PRIORITY) {
		printf(ERROR_PREFIX "The priority is not correct\n");
		exit(PTS_UNRESOLVED);
	}

	/* acquire the mutex */
	rc = pthread_mutex_lock(&mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_lock\n");
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
	pthread_exit((void *) 0);
}

void *low_priority_thread(void *tmp)
{
	struct timespec           current_time, start_time;
	struct sched_param        param;
	int                       rc = 0;
	int                       policy;

	param.sched_priority = LOW_PRIORITY;
	rc = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "Policy not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != LOW_PRIORITY) {
		printf(ERROR_PREFIX "Priority not correct\n");
		exit(PTS_UNRESOLVED);
	}

	/* Grab the start time and busy loop for 5 seconds */
	clock_gettime(CLOCK_REALTIME, &start_time);
	while (1) {
		clock_gettime(CLOCK_REALTIME, &current_time);
		if (timediff(current_time, start_time) > RUNTIME)
			break;
	}
	low_done = 1;
	pthread_exit((void *) 0);
}

int main()
{
	pthread_t                high_id, low_id;
	pthread_attr_t           low_attr, high_attr;
	struct sched_param       param;
	int                      rc = 0;
	int                      policy;

	param.sched_priority = MID_PRIORITY;
	rc =  pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	if (policy != SCHED_RR) {
		printf(ERROR_PREFIX "The policy is not correct\n");
		exit(PTS_UNRESOLVED);
	}
	if (param.sched_priority != MID_PRIORITY) {
		printf(ERROR_PREFIX "The priority is not correct\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_mutex_lock(&mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* create the higher priority */
	rc = pthread_attr_init(&high_attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_attr_setschedpolicy(&high_attr, SCHED_RR);
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
	rc = pthread_attr_setschedpolicy(&low_attr, SCHED_RR);
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

	/* setup a signal handler which will wakeup main later */
	if (signal(SIGALRM, signal_handler) == SIG_ERR) {
		perror(ERROR_PREFIX "signal");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_mutex_lock(&cond_mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_lock\n");
		exit(PTS_UNRESOLVED);
	}

	alarm(2);
	rc = pthread_cond_wait(&cond, &cond_mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_cond_wait\n");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_mutex_unlock(&cond_mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_unlock\n");
		exit(PTS_UNRESOLVED);
	}

	/* Wake the other high priority thread up */
	rc = pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_unlock\n");
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
		printf("High priority was not woken up. Test FAILED.\n");
		exit(PTS_FAIL);
	}
	printf("Test PASS\n");
	exit(PTS_PASS);
}