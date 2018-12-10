/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_timedwait()
 *   shall return ETIMEDOUT if the time specified by 'abstime' has passed.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT   3

struct testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} td;

void *t1_func(void *arg)
{
	int rc;
	struct timespec timeout;
	struct timeval curtime;

	(void) arg;

	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "Thread1 failed to acquire the mutex\n");
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "Thread1 started\n");

	if (gettimeofday(&curtime, NULL) != 0) {
		fprintf(stderr, "Fail to get current time\n");
		exit(PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec + TIMEOUT;
	timeout.tv_nsec = curtime.tv_usec * 1000;

	fprintf(stderr, "Thread1 is waiting for the cond for %d seconds\n",
		TIMEOUT);
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc == ETIMEDOUT) {
		fprintf(stderr, "Thread1 stops waiting when time is out\n");
		printf("Test PASSED\n");
		exit(PTS_PASS);
	} else {
		fprintf(stderr,
			"pthread_cond_timedwait return %d instead of ETIMEDOUT\n",
			rc);
		printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
}

int main(void)
{
	pthread_t thread1;
	void *th_ret;

	if (pthread_mutex_init(&td.mutex, NULL) != 0) {
		fprintf(stderr, "Fail to initialize mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_cond_init(&td.cond, NULL) != 0) {
		fprintf(stderr, "Fail to initialize cond\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&thread1, NULL, t1_func, NULL) != 0) {
		fprintf(stderr, "Fail to create thread 1\n");
		return PTS_UNRESOLVED;
	}

	fprintf(stderr, "Main: no condition is going to be met\n");

	pthread_join(thread1, &th_ret);
	return (long)th_ret;
}
