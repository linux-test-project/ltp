/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
 *
 *	It may fail if:
 *	[EINVAL] rwlock does not refer to an intialized read-write lock object
 *
 * Steps:
 * 1. Call pthread_rwlock_trywrlock with an uninitialized rwlock object
 * 2. Test for the return code.  It may be EINVAL or 0.
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

	/* Call without initializing rwlock */
	rc = pthread_rwlock_trywrlock(&rwlock);

	/* Clean up before checking return value */
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}

	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	if(rc != 0)
	{
		if(rc == EINVAL)
		{
			printf("Test PASSED\n");
			return PTS_PASS;
		} else
		{
			printf("Test FAILED: Incorrect return code %d\n", rc);
			return PTS_FAIL;
		}
	}
	
	printf("Test PASSED: Note*: Returned 0 instead of EINVAL, but standard specified _may_ fail. \n");
	return PTS_PASS;
}
