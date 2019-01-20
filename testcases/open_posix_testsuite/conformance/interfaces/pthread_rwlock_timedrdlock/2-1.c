/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock)
 *
 *	The timeout shall expire when the absolute time specified by abs_timeout passes,
 *	as measured by the clock on which timeouts are based (that is, when the
 *	value of that clock equals or exceeds abs_timeout), or if the absolute time
 *	specified by abs_timeout has already been passed at the time of the call.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'rwlock' for writing with pthread_rwlock_rdlock()
 * 3.  Create a child thread, specify a 'abs_timeout' as being the current time _minus_
 *     a timeout value of 1. (this ensures that the abs_timeout has already passed)
 * 4.  The thread lock 'rwlock' for reading, using pthread_rwlock_timedrdlock(). Should
 *	get an ETIMEOUT error.
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 1

static pthread_rwlock_t rwlock;
static int thread_state;
static time_t currsec1, currsec2;
static int expired;

static void *fn_rd(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct timespec abs_timeout;
	int rc;
	thread_state = ENTERED_THREAD;

	currsec1 = time(NULL);

	/* Absolute time, not relative. */
	abs_timeout.tv_sec = currsec1 - TIMEOUT;
	abs_timeout.tv_nsec = 0;

	printf("thread: attempt timed read-lock\n");
	rc = pthread_rwlock_timedrdlock(&rwlock, &abs_timeout);
	if (rc == ETIMEDOUT) {
		printf("thread: timed read-lock correctly expired\n");
		expired = 1;
	} else if (rc == 0) {
		printf("thread: acquired read-lock\n");
		expired = 0;
		printf("thread: unlock read lock\n");
		if (pthread_rwlock_unlock(&rwlock) != 0) {
			printf("thread: failed to release lock\n");
			exit(PTS_UNRESOLVED);
		}
	} else {
		printf
		    ("Error in pthread_rwlock_timedrdlock(), error code:%d.\n",
		     rc);
		exit(PTS_UNRESOLVED);
	}

	/* Get time after the mutex timed out in locking. */
	currsec2 = time(NULL);
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;
	pthread_t thread1;

	expired = 0;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf("Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	printf("main: create thread\n");
	thread_state = NOT_CREATED_THREAD;
	if (pthread_create(&thread1, NULL, fn_rd, NULL) != 0) {
		printf("Error creating thread1\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */

	/* We expect the thread _NOT_ to block, and instead time out */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 5);

	if (thread_state == EXITING_THREAD) {
		/* the child thread does not block, check the time expired or not */
		if (expired != 1) {
			printf("Test FAILED: abs_timeout should expire\n");
			exit(PTS_FAIL);
		}
	} else if (thread_state == ENTERED_THREAD) {
		printf
		    ("Test FAILED: thread blocked even when the timer expired\n");
		exit(PTS_FAIL);
	} else {
		printf("Unexpected thread state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock write lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_join(thread1, NULL) != 0) {
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("main: Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
