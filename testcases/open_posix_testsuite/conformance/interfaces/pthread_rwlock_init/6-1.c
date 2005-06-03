/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * 
 * Test pthread_rwlock_init().
 *
 * 	May fail if:
 *	[EBUSY] The implementation has detected an attempt to reinitialize the object
 *	referenced by rwlock, a previously initialized but not yet destroyed read-write
 *	lock.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init().
 * 2.  Re-initialize it again without destroying it first.
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

	/* Initialize the rwlock */
	rc = pthread_rwlock_init(&rwlock, NULL);
	if(rc != 0)
	{
		printf("Test FAILED: Error at pthread_rwlock_init(), returns %d\n", rc);
		return PTS_FAIL;
	}
	
	/* Re-intialize without destroying it first */
	rc = pthread_rwlock_init(&rwlock, NULL);
	
	/* Cleanup */	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	
	if(rc == EBUSY)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (rc == 0)
	{
		printf("Test PASSED: Note*: pthread_rwlock_init() returned 0 instead of EBUSY, but standard specifies _may_ fail\n");
		return PTS_PASS;
	} else 
	{
		printf("Test FAILED: Error at pthread_rwlock_init(), should return 0 or EBUSY, but returns %d\n", rc);
		return PTS_FAIL;
	}
		
}
