/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setpshared()
 * 
 *  It shall set the process-shared attribute in an initialized attributes object 'attr'.

 * The process-shared attribute is set to PTHREAD_PROCESS_SHARED to permit a mutex to be
 * operated upon by any thread that has access to the memory where the mutex is allocated,
 * even if the mutex is allocated in memory that is shared by multiple processes.
 *
 * If the process-shared attribute is PTHREAD_PROCESS_PRIVATE, the mutex shall only be 
 * operated upon by threads created within the same process as the thread that initialized
 * the mutex; if threads of differing processes attempt to operate on such a mutex,
 * the behavior is undefined.
 *
 * Steps:
 *
 * Explanation:  To share a mutex between 2 processes, you need to map shared memory for
 * the mutex.  So whether the 'type' of the mutexattr is shared or private, it really will
 * not make a difference since both processes will always have access to the shared memory
 * as long as they the pointer to it.  So all we check here is that you can actually call
 * the pthread_mutexattr_setpshared() function, passing to it PTHREAD_PROCESS_SHARED and
 * PTHREAD_PROCESS_PRIVATE.
 *
 * 1. Initialize a pthread_mutexattr_t object.
 * 2. Call pthread_mutexattr_getpshared(), passing to it both PTHREAD_PROCESS_SHARE and
 *    PTHREAD_PROCESS_PRIVATE.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

pthread_mutex_t new_mutex;	/* The mutex. */


int main()
{
	pthread_mutexattr_t mta;
	int ret;

	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Set the 'pshared' attribute to PTHREAD_PROCESS_PRIVATE */
	if((ret=pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_PRIVATE)) != 0)
	{
		printf("Test FAILED: Cannot set pshared attribute to PTHREAD_PROCESS_PRIVATE. Error: %d\n", ret);
		return PTS_FAIL;
	}

	/* Set the 'pshared' attribute to PTHREAD_PROCESS_SHARED */
	if((ret=pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_SHARED)) != 0)
	{
		printf("Test FAILED: Cannot set pshared attribute to PTHREAD_PROCESS_SHARED. Error code: %d\n", ret);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
