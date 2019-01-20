/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
 *
 * 	pthread_rwlock_unlock() function shall release a lock held on the
 *	read-write lock object referenced by rwlock
 *	If this function is called to release a read lock from the read-write lock object
 *	and there are other read locks currently held on this read-write lock object,
 *	the read-write lock object remains in the read locked state.
 *	If this function releases the last read lock for this read-write lock object,
 *	the read-write lock object shall be put in the unlocked state with no owners.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread read lock 'rwlock'
 * 3.  Create a child thread, the thread read lock 'rwlock', should not block
 * 4.  Child thread unlock 'rwlock', while the main thread still hold the read lock.
 * 5.  Child thread read lock 'rwlock' again, should succeed, then unlock again
 * 6.  Child thread write lock 'rwlock', should block
 * 7.  Main thread unlock the read lock, the 'rwlock' is in unlocked state
 * 8.  Child thread should get the lock for writing.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int thread_state;

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: after 1st read lock
	4: after 2nd read lock and before write lock;
	5: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define PASSED_RLOCK1 3
#define PASSED_RLOCK2 4
#define EXITING_THREAD 5

static void *fn_rd(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc = 0;

	thread_state = ENTERED_THREAD;
	printf("thread: attempt 1st read lock\n");
	if (pthread_rwlock_rdlock(&rwlock) != 0) {
		printf("thread: cannot get read lock\n");
		exit(PTS_UNRESOLVED);
	}
	printf("thread: acquired read lock\n");
	printf("thread: unlock read lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: thread: Error at pthread_rwlock_unlock(), Error Code=%d\n",
		     rc);
		exit(PTS_FAIL);
	}

	thread_state = PASSED_RLOCK1;
	printf("thread: attempt 2nd read lock\n");
	if (pthread_rwlock_rdlock(&rwlock) != 0) {
		printf("thread: cannot get read lock\n");
		exit(PTS_UNRESOLVED);
	}
	printf("thread: acquired read lock\n");
	printf("thread: unlock read lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: thread: Error at 2nd pthread_rwlock_unlock(), Error Code=%d\n",
		     rc);
		exit(PTS_FAIL);
	}

	thread_state = PASSED_RLOCK2;
	/* The thread should block here */
	printf("thread: attempt write lock\n");
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf("thread: cannot get write lock\n");
		exit(PTS_UNRESOLVED);
	}
	printf("thread: acquired write lock\n");
	printf("thread: unlock write lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: thread failed to release write lock, Error Code=%d\n",
		     rc);
		exit(PTS_FAIL);
	}
	thread_state = EXITING_THREAD;
	return NULL;
}

int main(void)
{
	int cnt = 0;
	int rc = 0;

	pthread_t rd_thread;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");

	/* This read lock should succeed */
	if (pthread_rwlock_rdlock(&rwlock) != 0) {
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}

	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if (pthread_create(&rd_thread, NULL, fn_rd, NULL) != 0) {
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != PASSED_RLOCK2 && cnt++ < 3);

	if (thread_state == ENTERED_THREAD) {
		printf("Thread should not block on first read lock\n");
		exit(PTS_UNRESOLVED);
	} else if (thread_state == PASSED_RLOCK1) {
		/* child thread blocked on second read lock */
		printf("thread should not block on second read lock\n");
		exit(PTS_UNRESOLVED);
	} else if (thread_state != PASSED_RLOCK2) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	/* thread_state == 4, i.e. child thread blocks on write lock */
	printf("main: unlock read lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if (rc != 0) {
		printf("Test FAILED: Main cannot release read lock\n");
		exit(PTS_FAIL);
	}

	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state != EXITING_THREAD) {
		printf
		    ("Test FAILED: thread did not get write lock even when the lock has no owner\n");
		exit(PTS_FAIL);
	}

	if (pthread_join(rd_thread, NULL) != 0) {
		printf("Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
