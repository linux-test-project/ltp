/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
 *
 * The function shall apply a write lock to the read-write lock referenced by
 * 'rwlock'. The calling thread acquires the write lock if no other thread
 * (reader or writer) holds the read-write lock 'rwlock'. Otherwise, the thread
 * shall block until it can acquire the lock.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'rwlock' for reading with pthread_rwlock_rdlock()
 * 3.  Create a child thread, the thread lock 'rwlock' for writing, shall block
 * 4.  Main thread unlock 'rwlock', child thread should get the write lock
 * 5.  Main thread lock 'rwlock' for writing
 * 6.  Create child thread to lock 'rwlock' for writing, should block
 * 7.  Main thread unlock 'rwlock'
 * 8.  Child got the lock
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int thread_state;

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void *fn_wr(void *arg LTP_ATTRIBUTE_UNUSED)
{

	thread_state = ENTERED_THREAD;
	int rc;

	printf("thread: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&rwlock);
	if (rc == 0) {
		printf("thread: acquired write lock\n");
		printf("thread: unlock write lock\n");
		if (pthread_rwlock_unlock(&rwlock) != 0) {
			printf("thread1: Error at pthread_rwlock_unlock()\n");
			exit(PTS_UNRESOLVED);
		}
	} else {
		printf
		    ("Test FAILED: Error in pthread_rwlock_wrlock(), error code: %d\n",
		     rc);
		exit(PTS_FAIL);
	}

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;
	pthread_t thread1, thread2;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */
	if (pthread_rwlock_rdlock(&rwlock) != 0) {
		printf("Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: acquired read lock\n");
	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread1\n");
	if (pthread_create(&thread1, NULL, fn_wr, NULL) != 0) {
		printf("Error creating thread1\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == EXITING_THREAD) {
		/* the child thread did not block */
		printf("Test FAILED: The thread1 did not block\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		/* for some reason the child thread did not start */
		printf("Child thread in unexpected state\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock read lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("Error releasing read lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	/* thread1 should get the write lock */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == ENTERED_THREAD) {
		/* the child thread should get the lock */
		printf("Test FAILEd: thread1 did not get the write lock\n");
		exit(PTS_FAIL);
	} else if (thread_state != EXITING_THREAD) {
		/* for some reason the child does not start */
		printf("Child thread1 in unexpected state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(thread1, NULL) != 0) {
		printf("Error joining thread1\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: attempt write lock\n");
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf("Test FAILED: main: Error geting write lock\n");
		return PTS_FAIL;
	}

	printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create thread2\n");
	if (pthread_create(&thread2, NULL, fn_wr, NULL) != 0) {
		printf("main: Error creating thread2\n");
		return PTS_UNRESOLVED;
	}

	/* Thread2 should block */
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == EXITING_THREAD) {
		printf("Test FAILED: thread2 should have block\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		printf("thread2 in unexpected state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock write lock\n");

	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: Error at releasing write lock\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(thread2, NULL) != 0) {
		printf("main: Error joining thread2\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
