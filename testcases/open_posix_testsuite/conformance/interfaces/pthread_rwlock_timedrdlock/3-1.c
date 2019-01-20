/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock)
 *
 *  The function shall apply a read lock to the read-write lock referenced by
 *  rwlock as in the pthread_rwlock_rdlock(). However, if the lock cannot be
 *  acquired with out waiting for other threads to unlock the lock, this wait
 *  shall be terminated when the specified timeout expires.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'rwlock' for reading with pthread_rwlock_rdlock()
 * 3.  Create a child thread, the thread lock 'rwlock' for reading,
 *     using pthread_rwlock_timedrdlock(), should get read lock. Thread unlocks 'rwlock'.
 * 4.  Main thread unlock 'rwlock'
 * 5.  Main thread lock 'rwlock' for writing
 * 6.  Create child thread to lock 'rwlock' for reading,
 *     using pthread_rwlock_timedrdlock, should block
 *     but when the timer expires, the wait will be terminated
 * 7.  Main thread unlock 'rwlock'
 */

/* Test for CLOCK_REALTIME */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 3

static pthread_rwlock_t rwlock;
static int thread_state;
static struct timeval currsec1, currsec2;

static void *fn_rd(void *arg LTP_ATTRIBUTE_UNUSED)
{

	thread_state = ENTERED_THREAD;
	struct timespec timeout, ts;
	int rc;
#ifdef CLOCK_REALTIME
	printf("Test CLOCK_REALTIME\n");
	clock_gettime(CLOCK_REALTIME, &ts);
	currsec1.tv_sec = ts.tv_sec;
	currsec1.tv_usec = ts.tv_nsec / 1000;
#else
	gettimeofday(&currsec1, NULL);
#endif
	/* Absolute time, not relative. */
	timeout.tv_sec = currsec1.tv_sec + TIMEOUT;
	timeout.tv_nsec = currsec1.tv_usec * 1000;

	printf("thread: attempt timed read lock, %d secs\n", TIMEOUT);
	rc = pthread_rwlock_timedrdlock(&rwlock, &timeout);
	if (rc == ETIMEDOUT)
		printf("thread: timer expired\n");
	else if (rc == 0) {
		printf("thread: acquired read lock\n");
		printf("thread: unlock read lock\n");
		if (pthread_rwlock_unlock(&rwlock) != 0) {
			exit(PTS_UNRESOLVED);
		}
	} else {
		printf
		    ("Error: thread: in pthread_rwlock_timedrdlock(), return code:%d\n",
		     rc);
		exit(PTS_UNRESOLVED);
	}

	/* Get time after the pthread_rwlock_timedrdlock() call. */
#ifdef CLOCK_REALTIME
	clock_gettime(CLOCK_REALTIME, &ts);
	currsec2.tv_sec = ts.tv_sec;
	currsec2.tv_usec = ts.tv_nsec / 1000;
#else
	gettimeofday(&currsec2, NULL);
#endif
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;
	pthread_t rd_thread1, rd_thread2;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");
	if (pthread_rwlock_rdlock(&rwlock) != 0) {
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired read lock\n");

	thread_state = NOT_CREATED_THREAD;

	printf("main: create rd_thread1\n");
	if (pthread_create(&rd_thread1, NULL, fn_rd, NULL) != 0) {
		printf("main: Error when creating rd_thread1\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */

	/* we expect the thread not to block */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 5);

	if (thread_state == ENTERED_THREAD) {
		/* the child thread started but blocked */
		printf
		    ("Test FAILED: rd_thread1 blocked on pthread_rwlock_timedrdlock()\n");
		exit(PTS_FAIL);
	} else if (thread_state != EXITING_THREAD) {
		printf("Unexpected thread state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(rd_thread1, NULL) != 0) {
		printf("main: Error when join rd_thread1\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock read lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: Error when release read lock\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf("main: Failed to get write lock\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	printf("main: create rd_thread2\n");
	if (pthread_create(&rd_thread2, NULL, fn_rd, NULL) != 0) {
		printf("main: Failed to create rd_thread2\n");
		return PTS_UNRESOLVED;
	}

	/* we expect rd_thread2 to block and timeout. */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 5);

	if (thread_state == EXITING_THREAD) {
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = currsec2.tv_sec - currsec1.tv_sec;
		time_diff.tv_usec = currsec2.tv_usec - currsec1.tv_usec;
		if (time_diff.tv_usec < 0) {
			--time_diff.tv_sec;
			time_diff.tv_usec += 1000000;
		}
		if (time_diff.tv_sec < TIMEOUT) {
			printf
			    ("Test FAILED: the timer expired and thread terminated, "
			     "but the timeout is not correct: "
			     "start time %ld.%06ld, end time %ld.%06ld\n",
			     (long)currsec1.tv_sec, (long)currsec1.tv_usec,
			     (long)currsec2.tv_sec, (long)currsec2.tv_usec);
			exit(PTS_FAIL);
		} else
			printf("thread: read lock correctly timed out\n");
	} else if (thread_state == ENTERED_THREAD) {
		printf
		    ("Test FAILED: read block was not terminated even when the timer expired\n");
		exit(PTS_FAIL);
	} else {
		printf("Unexpected thread state %d\n", thread_state);
		return PTS_UNRESOLVED;
	}

	printf("main: unlock write lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: Failed to release write lock\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlockattr_destroy()\n");
		exit(PTS_UNRESOLVED);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
