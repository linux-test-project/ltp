/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_spin_lock(pthread_spinlock_t *lock)
 *
 * 35543 These functions may fail if:
 * 35544 [EINVAL] The value specified by lock does not
 * refer to an initialized spin lock object.
 *
 * This case will always pass.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int rc;

static void sig_handler()
{
	printf("main: interrupted by SIGALARM\n");
	if (rc == EINVAL) {
		printf("main: correctly got EINVAL\n");
		printf("Test PASSED\n");
	} else {
		printf("main: get return code: %d, %s\n", rc, strerror(rc));
		printf
		    ("Test PASSED: *Note: Did not return EINVAL when attempting to lock an uninitialized spinlock, but standard says 'may' fail\n");
	}

	exit(PTS_PASS);
}

int main(void)
{
	pthread_spinlock_t spinlock;
	struct sigaction act;

	/* Set up child thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	printf("main: attemp to lock an un-initialized spin lock\n");

	printf("main: Send SIGALRM to me after 5 secs\n");
	alarm(5);

	/* Attempt to lock an uninitialized spinlock */
	rc = pthread_spin_lock(&spinlock);

	/* If we get here, call sig_handler() to check the return code of
	 * pthread_spin_lock() */
	sig_handler();

	/* Unlock spinlock */
	pthread_spin_unlock(&spinlock);

	/* Destroy spinlock */
	pthread_spin_destroy(&spinlock);

	return PTS_PASS;
}
