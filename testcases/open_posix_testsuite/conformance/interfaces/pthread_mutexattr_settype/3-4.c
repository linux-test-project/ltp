/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutexattr_settype()
 *
  PTHREAD_MUTEX_ERRORCHECK

  Provides errorchecking.  A thread attempting to relock this mutex without unlocking it 
  first will return with an error.  A thread attempting to unlock a mutex which another
  thread has locked will return with an error.  A thread attempting to unlock an unlocked
  mutex will return with an error.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2   Set the 'type' of the mutexattr object to PTHREAD_MUTEX_ERRORCHECK.
 * 3.  Create a mutex with that mutexattr object.
 * 4.  Attempt to unlock an unlocked mutex.  It should return an error.
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
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* Set the 'type' attribute to be PTHREAD_MUTEX_ERRORCHECK  */
	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK) != 0)
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

	if(pthread_mutex_lock(&mutex) != 0)
	{
		perror("Error at pthread_mutex_lock().\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_mutex_unlock(&mutex) != 0)
	{
		perror("Error at pthread_mutex_unlock().\n");
		return PTS_UNRESOLVED;
	}
	
	/* Unlock an already unlocked mutex.  Here, an error should be returned. */
	if(pthread_mutex_unlock(&mutex) == 0 )
	{
		perror("Test FAILED: Did not return error when unlocking an already unlocked mutex.\n");
		return PTS_FAIL;
	}
	
	if(pthread_mutex_destroy(&mutex))
	{
		perror("Error at pthread_mutex_destory().\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_mutexattr_destroy(&mta))
	{
		perror("Error at pthread_mutexattr_destory().\n");
		return PTS_UNRESOLVED;
	}	
		
	printf("Test PASSED\n");
	return PTS_PASS;
}
