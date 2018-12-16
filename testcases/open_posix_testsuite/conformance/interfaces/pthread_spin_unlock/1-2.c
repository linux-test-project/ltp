/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test pthread_spin_unlock(pthread_spinlock_t *lock)
 *
 * The function shall release the spin lock referenced by 'lock' which
 * was locked via the pthread_spin_lock() or pthread_spin_trylock().
 *
 * CAUTION: If setting the priority of the process running this case
 * higher than other process in the system, the system might become
 * unresponsive. The child will spin on the spinlock, no other process
 * can interupt it.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Create a child thread. The thread lock 'spinlock' with pthread_spin_lock()
 *     should spin on the lock.
 * 4.  Main thread unlock 'spinlock'
 * 5.  Child thread should get 'spinlock'
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
static volatile int thread_state;

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void *fn_chld(void *arg)
{
	int rc = 0;
	thread_state = ENTERED_THREAD;

	(void) arg;

	printf("thread: attempt spin lock\n");
	rc = pthread_spin_lock(&spinlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: thread failed to get spin lock error code:%d\n",
		     rc);
		exit(PTS_FAIL);
	}
	printf("thread: acquired spin lock\n");

	sleep(1);

	printf("thread: unlock spin lock\n");
	if (pthread_spin_unlock(&spinlock)) {
		printf("Test FAILED: Error at pthread_spin_unlock()\n");
		exit(PTS_FAIL);
	}

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;

	pthread_t child_thread;

	if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */
	if (pthread_spin_lock(&spinlock) != 0) {
		printf
		    ("Error: main cannot get spin lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired spin lock\n");

	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if (pthread_create(&child_thread, NULL, fn_chld, NULL) != 0) {
		printf("main: Error creating child thread\n");
		return PTS_UNRESOLVED;
	}

	cnt = 0;
	/* Expect the child thread to spin on spin lock */
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == EXITING_THREAD) {
		printf
		    ("Test FAILED: child thread did not spin on spin lock when other thread holds the lock\n");
		return PTS_FAIL;
	} else if (thread_state != ENTERED_THREAD) {
		printf("main: Unexpected thread state %d\n", thread_state);
		return PTS_UNRESOLVED;
	}

	printf("main: unlock spin lock\n");
	if (pthread_spin_unlock(&spinlock) != 0) {
		printf("Test FAILED: main: Error at pthread_spin_unlock()\n");
		return PTS_FAIL;
	}

	/* We expected the child get the spin lock and exit */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == ENTERED_THREAD) {
		printf("Test FAILED: child thread did not get spin lock\n");
		return PTS_FAIL;
	} else if (thread_state != EXITING_THREAD) {
		printf("main: Unexpected thread state %d\n", thread_state);
		return PTS_UNRESOLVED;
	}

	if (pthread_join(child_thread, NULL) != 0) {
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_spin_destroy(&spinlock) != 0) {
		printf("Error at pthread_spin_destroy()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
