/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * The function shall destroy the read-write lock object referenced by rwlock
 * and release any resources used by the lock.
 *
 * Note: This case does not test the resources used by the lock has been released.
 *Steps:
 * 	Loop for COUNT times:
 * 	1. Initialize a read-write lock
 *	2. Destroy it. Should get not error.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

#define COUNT 1000

int main()
{
	pthread_rwlock_t rwlock;
	int cnt = 0;
	int rc = 0;
	
	while(cnt++ < COUNT)
	{ 
		if(pthread_rwlock_init(&rwlock, NULL) != 0)
		{
				printf("Error at pthread_rwlock_init()\n");
				return PTS_UNRESOLVED;
		}
		
		rc = pthread_rwlock_destroy(&rwlock);
		if(rc != 0)
		{
			printf("Test FAILED: at %d-th pthread_rwlock_destroy(), with Error code=%d\n", cnt, rc);
			return PTS_FAIL;
		}
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
