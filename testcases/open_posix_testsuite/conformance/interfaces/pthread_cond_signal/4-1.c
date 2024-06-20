/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_signal()
 *   Upon successful completion, a value of zero shall be returned.
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "posixtest.h"

#define THREAD_NUM  5

static struct testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} td;

static pthread_t thread[THREAD_NUM];

static int start_num = 0;
static int waken_num = 0;

/* Alarm handler */
static void alarm_handler(int signo PTS_ATTRIBUTE_UNUSED)
{
	int i;
	printf("Error: failed to wakeup all threads\n");
	for (i = 0; i < THREAD_NUM; i++) {	/* cancel threads */
		pthread_cancel(thread[i]);
	}

	exit(PTS_UNRESOLVED);
}

static void *thr_func(void *arg PTS_ATTRIBUTE_UNUSED)
{
	int rc;
	pthread_t self = pthread_self();

	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "[Thread 0x%p] failed to acquire the mutex\n",
			(void *)self);
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "[Thread 0x%p] started\n", (void *)self);
	start_num++;

	fprintf(stderr, "[Thread 0x%p] is waiting for the cond\n",
		(void *)self);
	rc = pthread_cond_wait(&td.cond, &td.mutex);
	if (rc != 0) {
		fprintf(stderr, "pthread_cond_wait return %d\n", rc);
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "[Thread 0x%p] was wakened\n", (void *)self);
	waken_num++;

	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr, "[Thread 0x%p] failed to release the mutex\n",
			(void *)self);
		exit(PTS_UNRESOLVED);
	}
	return NULL;
}

int main(void)
{
	struct timespec completion_wait_ts = {0, 100000};
	int i, rc;
	struct sigaction act;

	waken_num = 0;
	start_num = 0;

	if (pthread_mutex_init(&td.mutex, NULL) != 0) {
		fprintf(stderr, "Fail to initialize mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_cond_init(&td.cond, NULL) != 0) {
		fprintf(stderr, "Fail to initialize cond\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_create(&thread[i], NULL, thr_func, NULL) != 0) {
			fprintf(stderr, "Fail to create thread[%d]\n", i);
			exit(PTS_UNRESOLVED);
		}
	}
	while (start_num < THREAD_NUM)	/* waiting for all threads started */
		nanosleep(&completion_wait_ts, NULL);

	/* Setup alarm handler */
	act.sa_handler = alarm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	alarm(5);

	while (waken_num < THREAD_NUM) {	/* loop to wake up all waiter threads */
		fprintf(stderr, "[Main thread] signals a condition\n");
		rc = pthread_cond_signal(&td.cond);
		if (rc == EINVAL) {
			fprintf(stderr, "pthread_cond_signal returns EINVAL\n");
			exit(PTS_UNRESOLVED);
		} else if (rc != 0) {
			fprintf(stderr, "pthread_cond_signal returns %d\n", rc);
			printf("Test FAILED\n");
			exit(PTS_FAIL);
		}
		nanosleep(&completion_wait_ts, NULL);
	}

	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			fprintf(stderr, "Fail to join thread[%d]\n", i);
			exit(PTS_UNRESOLVED);
		}
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
