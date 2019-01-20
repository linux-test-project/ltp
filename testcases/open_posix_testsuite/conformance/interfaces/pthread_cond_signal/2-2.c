/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_signal()
 *   When each thread unblocked as a result of pthread_cond_signal()
 *   returns from its call to pthread_cond_timedwait(), the thread shall
 *   own the mutex with which it called pthread_cond_timedwait().
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define THREAD_NUM  3
#define TIMEOUT     THREAD_NUM * 2

struct testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} td;

int start_num = 0;
int waken_num = 0;

void *thr_func(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc;
	struct timespec timeout;
	struct timeval curtime;
	pthread_t self = pthread_self();

	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "[Thread 0x%p] failed to acquire the mutex\n",
			(void *)self);
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "[Thread 0x%p] started and locked the mutex\n",
		(void *)self);
	start_num++;

	if (gettimeofday(&curtime, NULL) != 0) {
		fprintf(stderr, "Fail to get current time\n");
		exit(PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec + TIMEOUT;
	timeout.tv_nsec = curtime.tv_usec * 1000;

	fprintf(stderr,
		"[Thread 0x%p] is waiting for the cond for at most %d secs\n",
		(void *)self, TIMEOUT);
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc != 0) {
		fprintf(stderr, "[Thread 0x%p] pthread_cond_wait returned %d\n",
			(void *)self, rc);
		exit(PTS_UNRESOLVED);
	}

	if (pthread_mutex_trylock(&td.mutex) != 0) {
		fprintf(stderr,
			"[Thread 0x%p] should be able to lock the recursive mutex again\n",
			(void *)self);
		printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
	fprintf(stderr,
		"[Thread 0x%p] was wakened and acquired the mutex again\n",
		(void *)self);
	waken_num++;

	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr, "[Thread 0x%p] failed to release the mutex\n",
			(void *)self);
		printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr,
			"[Thread 0x%p] did not owned the mutex after the cond wait\n",
			(void *)self);
		printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
	fprintf(stderr, "[Thread 0x%p] released the mutex\n", (void *)self);
	return NULL;
}

int main(void)
{
	struct timespec completion_wait_ts = {0, 100000};
	int i;
	pthread_t thread[THREAD_NUM];
	pthread_mutexattr_t ma;

	if (pthread_mutexattr_init(&ma) != 0) {
		fprintf(stderr, "Fail to initialize mutex attribute\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE) != 0) {
		fprintf(stderr, "Fail to set the mutex attribute\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_mutex_init(&td.mutex, &ma) != 0) {
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
	while (start_num < THREAD_NUM)	/* waiting for all threads started */
		nanosleep(&completion_wait_ts, NULL);

	sleep(1);

	while (waken_num < THREAD_NUM) {	/* waiting for all threads wakened */
		fprintf(stderr, "[Main thread] signals a condition\n");
		if (pthread_cond_signal(&td.cond) != 0) {
			fprintf(stderr,
				"Main failed to signal the condition\n");
			return PTS_UNRESOLVED;
		}
		sleep(1);
	}

	for (i = 0; i < THREAD_NUM; i++) {
		if (pthread_join(thread[i], NULL) != 0) {
			fprintf(stderr, "Fail to join thread[%d]\n", i);
			return PTS_UNRESOLVED;
		}
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
