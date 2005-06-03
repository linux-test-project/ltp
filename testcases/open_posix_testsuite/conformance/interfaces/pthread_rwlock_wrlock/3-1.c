/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
 *
 *	It may fail if:
 *	[EDEADLK] The current thread already owns the rwlock for writing or reading.
 *
 * Steps:
 * 1. Create and initialize an rwlock
 * 2. Perform a write lock via pthread_rwlock_wrlock()
 * 3. Perform a write lock _again_ with pthread_rwlock_wrlock without first unlocking rwlock()
 * 4. Test if returns EDEADLK or not.  Note the standard states "may" fail, so the test always
 *    passes even if 0 is returned.
 * 
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	static pthread_rwlock_t rwlock;
	int rc;
	
	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Attempt to write lock rwlock, it should return successfully */	
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");	

	/* Now attempt to write lock again without first unlocking rwlock. It may return
	 * EDEADLK, but it may also return successfully. */

	printf("main: attempt write lock\n");
	rc = pthread_rwlock_wrlock(&rwlock);	
	
	/* Clean up before we test the return value of pthread_rwlock_wrlock() */
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("Error releasing write lock\n");
		exit(PTS_UNRESOLVED);
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	if(rc != 0)
	{
		if(rc == EDEADLK)
		{
			printf("main: correctly got EDEADLK\n");
			printf("Test PASSED\n");
			return PTS_PASS;
		} else
		{
			printf("Test FAILED: Incorrect return code %d\n", rc);
			return PTS_FAIL;
		}
	}
	
	printf("main: acquired write lock\n");	
	printf("main: unlock write lock\n");

	printf("Test PASSED: Note*: Returned 0 instead of EDEADLK, but standard specified _may_ fail.\n");
	return PTS_PASS;
}
