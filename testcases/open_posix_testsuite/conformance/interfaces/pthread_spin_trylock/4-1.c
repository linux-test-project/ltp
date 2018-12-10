/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_spin_trylock(pthread_spinlock_t *lock)
 *
 * Note: This case will always pass
 *
 * These functions may fail if:
 * [EINVAL] The value specified by lock does not refer to an initialized spin lock object.
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

int main(void)
{

	pthread_spinlock_t spinlock;
	int rc;

	/* attemp to lock an uninitalized spin lock */

	rc = pthread_spin_trylock(&spinlock);
	if (rc == EINVAL) {
		printf("Correctly got EINVAL at pthread_spin_trylock()\n");
		printf("Test PASSED\n");
	} else {
		printf("Expected EINVAL, but get return code: %d,%s\n", rc,
		       strerror(rc));
		printf
		    ("Test PASSED: *Note: Returned incorrect value, but standard says 'may' fail\n");
	}

	return PTS_PASS;
}
