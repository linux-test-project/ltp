/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test pthread_spin_init(pthread_spinlock_t * lock, int pshared)
 *
 * pthread_spin_init() shall allocate any resources required to use
 * the spin lock referenced by 'lock' and initialize the lock to an
 * unlocked state.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Main thread unlock 'spinlock'
 * 4.  Main thread destroy the 'spinlock'
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;

int main(void)
{
	int rc = 0;
	int pshared;

#ifdef PTHREAD_PROCESS_PRIVATE
	pshared = PTHREAD_PROCESS_PRIVATE;
#else
	pshared = -1;
#endif

	rc = pthread_spin_init(&spinlock, pshared);
	if (rc != 0) {
		printf("Test FAILED:  Error at pthread_spin_init(): %d\n", rc);
		return PTS_FAIL;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */
	if (pthread_spin_lock(&spinlock) != 0) {
		perror
		    ("Error: main cannot get spin lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	}

	printf("main: acquired spin lock\n");

	if (pthread_spin_unlock(&spinlock) != 0) {
		perror("main: Error at pthread_spin_unlock()\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_spin_destroy(&spinlock);
	if (rc != 0) {
		printf("Error at pthread_spin_destroy(): %d\n", rc);
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
