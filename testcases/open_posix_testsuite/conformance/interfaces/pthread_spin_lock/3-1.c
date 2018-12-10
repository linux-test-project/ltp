/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_spin_lock(pthread_spinlock_t *lock)
 *
 * The pthread_spin_lock() function may fail if:
 * [EDEADLK] The calling thread already holds the lock.
 *
 * This case will always pass. The thread might keep spin
 * when re-lock the spin lock.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

static void sig_handler()
{
	printf("main: interrupted by SIGALARM\n");
	printf
	    ("Test PASSED: *Note: Did not return EDEADLK when re-locking a spinlock already in  use, but standard says 'may' fail\n");
	pthread_exit((void *)PTS_PASS);
}

int main(void)
{
	int rc;
	pthread_spinlock_t spinlock;
	struct sigaction act;

	/* Set up child thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */
	if (pthread_spin_lock(&spinlock) != 0) {
		printf
		    ("Test FAILED: main cannot get spin lock when no one owns the lock\n");
		return PTS_FAIL;
	}
	printf("main: acquired spin lock\n");

	printf("main: send SIGALRM to me after 2 secs\n");
	alarm(2);

	printf("main: re-lock spin lock\n");
	rc = pthread_spin_lock(&spinlock);

	if (rc == EDEADLK) {
		printf
		    ("main: correctly got EDEADLK when re-locking the spin lock\n");
		printf("Test PASSED\n");
	} else {
		printf("main: get return code: %d , %s\n", rc, strerror(rc));
		printf
		    ("Test PASSED: *Note: Did not return EDEADLK when re-locking a spinlock already in  use, but standard says 'may' fail\n");
	}

	/* Unlock spinlock */
	pthread_spin_unlock(&spinlock);

	/* Destroy spinlock */
	pthread_spin_destroy(&spinlock);

	return PTS_PASS;
}
