/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * 
 * Test pthread_rwlock_init().
 *
 * 	Once initialized, the lock can be used any number of times without being 
 *	reinitialized.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Loop for COUNT time: lock for reading, unlock, lock for writing, unlock;
 */
  
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define COUNT 1000

static pthread_rwlock_t rwlock; 

 
int main()
{
	int cnt = 0;
	pthread_rwlockattr_t rwlockattr;	

	if(pthread_rwlockattr_init(&rwlockattr) != 0)
	{
		printf("Error at pthread_rwlockattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_init(&rwlock, &rwlockattr) != 0)
	{
		printf("Test FAILED: Error in pthread_rwlock_init()\n");
		return PTS_FAIL;
	}
	
	while(cnt++ < COUNT)
	{
		if(pthread_rwlock_rdlock(&rwlock) != 0)
		{
			printf("Test FAILED: cannot get read lock on %dth loop\n", cnt);
			return PTS_FAIL;
		}
		
		if(pthread_rwlock_unlock(&rwlock) != 0)
		{
			printf("Test FAILED: cannot release read lock on %dth loop\n", cnt);
			return PTS_FAIL;
		}
		
		if(pthread_rwlock_wrlock(&rwlock) != 0)
		{
			printf("Test FAILED: cannot get write lock on %dth loop\n", cnt);
			return PTS_FAIL;
		}
		
		if(pthread_rwlock_unlock(&rwlock) != 0)
		{
			printf("Test FAILED: cannot release write lock on %dth loop\n", cnt);
			return PTS_FAIL;
		}
	}	
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	if(pthread_rwlockattr_destroy(&rwlockattr) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;		
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
