/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_cancelstate
 * It returns a 0 on success.  It may return an error code of:
 *
 * [EINVAL] The specified state is not PTHREAD_CANCEL_ENABLE or
 * PTHREAD_CANCEL_DISABLE.

 * It will not return EINTR.
 *
 * STEPS:
 * 1. Create a thread.
 * 2. In the thread function, set the state to an invalid value -100;
 * 3. Check the return value.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int ret; 		/* Return value of pthread_setcancelstate(). */

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	/* Set cancel state to an invalid integer and save the return value. */
	ret=pthread_setcancelstate(-100, NULL);

	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
		
	/* Initializing value */
	ret=0;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to end execution. */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	/* This means that pthread_setcancelstate() did not give an error when passed an
	 * invalid state value of -100. */
	if(ret != EINVAL)
	{
		if(ret == 0)
		{
			printf("Test PASSED: *NOTE: Returned 0 on error, though standard states 'may' fail.\n");
			return PTS_PASS;
		}

		printf("Test FAILED: returned invalid error code of %d.\n", ret); 
		return PTS_FAIL;
	}	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


