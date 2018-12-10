/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_timedwait()
 *   shall be equivalent to pthread_cond_wait(), except that an error is returned
 *   if the absolute time specified by abstime has already been passed at the time
 *   of the call.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define INTERVAL  2

struct testdata {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} td;

int t1_start = 0;

void *t1_func(void *arg)
{
	int rc;
	struct timeval curtime;
	struct timespec timeout;

	(void) arg;

	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr, "Thread1 failed to acquire the mutex\n");
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr, "Thread1 started\n");
	t1_start = 1;		/* let main thread continue */

	if (gettimeofday(&curtime, NULL) != 0) {
		fprintf(stderr, "Fail to get current time\n");
		exit(PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec;
	timeout.tv_nsec = 0;

	fprintf(stderr, "Thread1 is waiting for the cond\n");
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc == ETIMEDOUT) {
		fprintf(stderr, "Thread1 stops waiting when time is out\n");
		pthread_exit((void *)PTS_PASS);
	} else {
		fprintf(stderr,
			"pthread_cond_timedwait return %d instead of ETIMEDOUT\n",
			rc);
		exit(PTS_FAIL);
	}
}

int main(void)
{
	pthread_t thread1;
	int rc;
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

	/* If the thread hasn't ended in 5 seconds, then most probably
	 * pthread_cond_timedwait is failing to function correctly. */
	alarm(5);

	/* Wait for thread to end execution. */
	if (pthread_join(thread1, (void *)&th_ret) != 0) {
		fprintf(stderr, "Could not join the thread. \n");
		return PTS_UNRESOLVED;
	}

	/* Make sure pthread_cond_timedwait released and re-acquired the mutex
	 * as it should. */
	rc = pthread_mutex_trylock(&td.mutex);
	if (rc == 0) {
		fprintf(stderr,
			"Test FAILED: Did not re-acquire mutex after timedout out call to pthread_cond_timedwait\n");
		return PTS_FAIL;
	}

	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr, "Main failed to release mutex\n");
		return PTS_UNRESOLVED;
	}

	switch ((long)th_ret) {
	case PTS_PASS:
		printf("Test PASSED\n");
		return PTS_PASS;
	case PTS_FAIL:
		printf("Test FAILED\n");
		return PTS_FAIL;
	default:
		return PTS_UNRESOLVED;
	}
}
