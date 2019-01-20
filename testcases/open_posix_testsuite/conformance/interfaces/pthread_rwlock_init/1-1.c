/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test pthread_rwlock_init().
 *
 *	pthread_rwlock_init() function shall allocate any resources
 *	required to use the read-write lock referenced by rwlock and
 *	initializes the lock to an unlocked state with attributes referenced
 *	by attr.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Create a child thread, the thread lock 'rwlock' for reading, shall not block.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int thread_state;

static void *fn_rd(void *arg LTP_ATTRIBUTE_UNUSED)
{

	thread_state = 2;
	int rc;

	printf("child: lock for reading\n");
	rc = pthread_rwlock_rdlock(&rwlock);
	if (rc == 0) {
		printf("child: get read lock\n");
		printf("child: unlock\n");
		if (pthread_rwlock_unlock(&rwlock) != 0) {
			printf("child: release read lock\n");
			exit(PTS_UNRESOLVED);
		}
	} else {
		printf("Error in pthread_rwlock_rdlock().\n");
		exit(PTS_FAIL);
	}

	thread_state = 3;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;
	int rc = 0;
	thread_state = 0;

	pthread_t thread;
	pthread_rwlockattr_t rwlockattr;

	if (pthread_rwlockattr_init(&rwlockattr) != 0) {
		printf("main: Error at pthread_rwlockattr_init()\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_rwlock_init(&rwlock, &rwlockattr);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error at pthread_rwlock_init(), returns %d\n",
		     rc);
		return PTS_FAIL;
	}

	thread_state = 1;
	printf("main: create thread\n");
	if (pthread_create(&thread, NULL, fn_rd, NULL) != 0) {
		printf("main: failed to create thread\n");
		return PTS_UNRESOLVED;
	}

	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	/* We expect the thread not to block */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != 3 && cnt++ < 3);

	if (thread_state == 2) {
		printf("Test FAILED: thread blocked on read lock\n");
		exit(PTS_FAIL);
	} else if (thread_state != 3) {
		printf("main: Unexpected thread state\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(thread, NULL) != 0) {
		printf("main: Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	/* Cleanup */
	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_rwlockattr_destroy(&rwlockattr) != 0) {
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
