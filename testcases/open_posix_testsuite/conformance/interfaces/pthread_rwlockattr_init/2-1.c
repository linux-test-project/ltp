/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  adam.li REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *	After a read-write lock attributes object has been used to initialize
 *	one or more read-write locks, any function affecting the attributes object
 *	(including destruction) shall not affect any previously
 *	initialized read-write locks.
 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object with pthread_rwlockattr_init()i
 * 2.  Initialize two pthread_rwlock_t objects with this pthread_rwlockattr_t object
 * 3.  Destruct this pthread_rwlockattr_t objects.
 * 4.  The two pthread_rwlock_t objects should safely be used to lock and unlock.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	pthread_rwlockattr_t rwa;
	pthread_rwlock_t rwl1, rwl2;
	int rc;

	/* Initialize a read-write lock attributes object */
	rc = pthread_rwlockattr_init(&rwa);
	if (rc != 0) {
		printf("Error at pthread_rwlockattr_init(), returns %d\n", rc);
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_rwlock_init(&rwl1, &rwa)) != 0) {
		printf("Error at 1st pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_rwlock_init(&rwl2, &rwa)) != 0) {
		printf("Error at 2nd pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_rwlock_rdlock(&rwl1)) != 0) {
		printf("Error at pthread_rwlock_rdlock(): rwl1\n");
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_rwlockattr_destroy(&rwa)) != 0) {
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}
	if ((rc = pthread_rwlock_unlock(&rwl1)) != 0) {
		printf("Error at pthread_rwlock_unlock: rwl1\n");
		printf("Test Failed.\n");
		return PTS_FAIL;
	}

	if ((rc = pthread_rwlock_rdlock(&rwl2)) != 0) {
		printf("Error at pthread_rwlock_rdlock():rwl2\n");
		printf("Test Failed.\n");
		return PTS_FAIL;
	}
	if ((rc = pthread_rwlock_unlock(&rwl2)) != 0) {
		printf("Error at pthread_rwlock_unlock:rwl2\n");
		printf("Test Failed.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
