/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test pthread_spin_unlock(pthread_spinlock_t *lock)
 *
 * This case will always PASS.
 *
 * The functions may fail if:
 * The pthread_spin_unlock() function may fail if:
 *	[EPERM] The calling thread does not hold the lock.
 *
 * Steps:
 * 1. Create a thread that will initialize and lock a spinlock
 * 2. Main will try to unlock the spinlock (it doesn't hold the lock)
 * 3. Check the return code to see if it returns [EPERM].
 *
 * Note: This test will always pass since the standard specifies 'may' fail.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
static volatile int sem;

#define INTHREAD 0
#define INMAIN 1

static void *fn_chld(void *arg)
{
	int rc = 0;

	(void) arg;

	/* Initialize spin lock */
	if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
		printf("main: Error at pthread_spin_init()\n");
		exit(PTS_UNRESOLVED);
	}

	/* Lock the spinlock */
	printf("thread: attempt spin lock\n");
	rc = pthread_spin_lock(&spinlock);
	if (rc != 0) {
		printf("Error: thread failed to get spin lock error code:%d\n",
		       rc);
		exit(PTS_UNRESOLVED);
	}
	printf("thread: acquired spin lock\n");

	/* Wait for main to try and unlock this spinlock */
	sem = INMAIN;
	while (sem == INMAIN)
		sleep(1);

	/* Cleanup just in case */
	pthread_spin_unlock(&spinlock);

	if (pthread_spin_destroy(&spinlock) != 0) {
		printf("Error at pthread_spin_destroy()");
		exit(PTS_UNRESOLVED);
	}

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int rc;
	pthread_t child_thread;

	sem = INTHREAD;

	/* Create a thread that will initialize and lock a spinlock */
	printf("main: create thread\n");
	if (pthread_create(&child_thread, NULL, fn_chld, NULL) != 0) {
		printf("main: Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to lock the spinlock */
	while (sem == INTHREAD)
		sleep(1);

	printf("main: attempt to unlock a spinlock that we don't own\n");
	rc = pthread_spin_unlock(&spinlock);
	if (rc != 0) {
		printf("main: Error at pthread_spin_unlock()\n");
		return PTS_FAIL;
	}

	/* Tell thread that we're done attempting to unlock the spinlock */
	sem = INTHREAD;

	/* Wait for thread to end execution */
	if (pthread_join(child_thread, NULL) != 0) {
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Test to see the return code of pthread_spin_unlock */
	if (rc == EPERM) {
		printf
		    ("main: correctly got EPERM when unlocking a spinlock we didn't have permission to unlock\n");
		printf("Test PASSED\n");
	} else {
		printf("main: got return code :%d\n", rc);
		printf
		    ("Test PASSED: *Note: Did not return EPERM when unlocking a spinlock it does not have a lock on, but standard says 'may' fail\n");
	}

	return PTS_PASS;
}
