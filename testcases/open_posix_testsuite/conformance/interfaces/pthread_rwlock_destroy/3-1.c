/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 *
 *	pthread_rwlock_destroy( ) function may fail if:
 *	[EBUSY] The implementation has detected an attempt to destroy the object referenced
 *	by rwlock while it is locked.
 *
 *Steps:
 * 	1. Initialize a read-write lock.
 *	2. Lock it.
 *	3. Attempt to destroy it while it is still locked.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main()
{
	pthread_rwlock_t rwlock;
	int rc;

	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_rdlock(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Attempt to destroy an already locked rwlock */
	rc = pthread_rwlock_destroy(&rwlock);
	if(rc == EBUSY)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (rc == 0)
	{
		printf("Test PASSED: Note*: pthread_rwlock_destroy() returned 0 instead of EBUSY, but standard specifies _may_ fail\n");
		return PTS_PASS;
	} else 
	{
		printf("Test FAILED: Error at pthread_rwlock_destroy(), should return 0 or EBUSY, but returns %d\n", rc);
		return PTS_FAIL;
	}
	
}
