/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
 *
 *	It 'may' fail if:
 *	[EINVAL]  rwlock doesn't refer to an initialized read-write lock
 *	[EPERM]  the current thread doesn't hold the lock on the rwlock
 *
 *	Testing EPERM in this test.
 *
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread read lock 'rwlock'
 * 3.  Create a child thread, the thread should try to unlock the 'rwlock'
 * 4. The test will pass even if it returns 0, but with a note stating that the standard
 *     states it 'may' fail.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int rc, thread_state;

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void *fn_unlk(void *arg LTP_ATTRIBUTE_UNUSED)
{
	thread_state = ENTERED_THREAD;
	printf("un_thread: unlock read lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	thread_state = EXITING_THREAD;
	return NULL;
}

int main(void)
{
	int cnt = 0;

	pthread_t un_thread;

#ifdef __linux__
	printf("Unlocking rwlock in different thread is undefined on Linux\n");
	return PTS_UNSUPPORTED;
#endif

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

	printf("main: create un_thread\n");
	if (pthread_create(&un_thread, NULL, fn_unlk, NULL) != 0) {
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for child to exit */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state != EXITING_THREAD) {
		printf("Unexpected thread state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(un_thread, NULL) != 0) {
		printf("Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	/* Cleaning up */
	pthread_rwlock_unlock(&rwlock);
	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}

	/* Test the return code of un_thread when it attempt to unlock the rwlock it didn't
	 * own in the first place. */

	if (rc != 0) {
		if (rc == EPERM) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf
		    ("Test FAILED: Incorrect error code, expected 0 or EPERM, got %d\n",
		     rc);
		return PTS_FAIL;
	}

	printf
	    ("Test PASSED: Note*: Returned 0 instead of EPERM, but standard specified _may_ fail.\n");
	return PTS_PASS;
}
