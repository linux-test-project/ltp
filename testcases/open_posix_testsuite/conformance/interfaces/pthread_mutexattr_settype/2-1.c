/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutexattr_settype()
 *
 *  PTHREAD_MUTEX_NORMAL

 *  This type of mutex doesn't detect deadlock.  So a thread attempting to relock this mutex
 *  without unlocking it first will not return an error.  Attempting to unlock a mutex locked 
 *  by a different thread results in undefined behavior.  Attemping to unlock an unlocked mutex
 *  results in undefined behavior. 
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2   Set the 'type' of the mutexattr object to PTHREAD_MUTEX_NORMAL.
 * 3.  Create a mutex with that mutexattr object.
 * 4.  Attempt to unlock the mutex without first locking it.  It shouldn't return an error.
 * 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mta;
	int ret;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* Set the 'type' attribute to be PTHREAD_MUTEX_NORMAL  */
	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL) != 0)
	{
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}

	/* Initialize the mutex with that attribute obj. */	
	if(pthread_mutex_init(&mutex, &mta) != 0)
	{
		perror("Error intializing the mutex.\n");
		return PTS_UNRESOLVED;
	}

	/* Attempt to unlock the mutex without first locking it.  It shouldn't return 
	 * an error. */
	ret=pthread_mutex_unlock(&mutex);
	if(ret != 0)
	{
		printf("Test FAILED: returned error on deadlock, error code %d\n", ret);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
