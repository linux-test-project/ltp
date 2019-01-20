/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_broadcast()
 *   Upon successful completion, a value of zero shall be returned.
 */


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define THREAD_NUM  5

struct testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} td;

static int start_num;
static int waken_num;

static void *thr_func(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc;
	pthread_t self = pthread_self();

	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "[Thread 0x%p] failed to acquire the mutex\n",
			(void *)self);
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "[Thread 0x%p] started and locked the mutex\n",
		(void *)self);
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
	pthread_t thread[THREAD_NUM];

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
			return PTS_UNRESOLVED;
		}
	}
	while (start_num < THREAD_NUM)
		nanosleep(&completion_wait_ts, NULL);

	/* Acquire the mutex to make sure that all waiters are currently
	   blocked on pthread_cond_wait */
	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "Main: Fail to acquire mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr, "Main: Fail to release mutex\n");
		return PTS_UNRESOLVED;
	}
	sleep(1);

	/* broadcast the condition to wake up all waiters */
	fprintf(stderr, "[Main thread] broadcast the condition\n");
	rc = pthread_cond_broadcast(&td.cond);
	if (rc != 0) {
		if (rc == EINVAL) {
			fprintf(stderr, "pthread_cond_broadcast "
				"returns EINVAL\n");
			return PTS_UNRESOLVED;
		} else if (rc != 0) {
			fprintf(stderr, "pthread_cond_broadcast returns %d\n",
				rc);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	fprintf(stderr, "[Main thread] pthread_cond_broadcast() returned 0\n");
	sleep(1);
	if (waken_num < THREAD_NUM) {
		fprintf(stderr, "[Main thread] Not all waiters were wakened\n");
		for (i = 0; i < THREAD_NUM; i++)
			pthread_cancel(thread[i]);

		return PTS_UNRESOLVED;
	}
	fprintf(stderr, "[Main thread] all waiters were wakened\n");

	/* join all secondary threads */
	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			fprintf(stderr, "Fail to join thread[%d]\n", i);
			return PTS_UNRESOLVED;
		}
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
