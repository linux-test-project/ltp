/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test If 'attr' is NULL, the default attributes shall be used.
 *  The attribute that will be tested is the detached state, because that is
 *  the only state that has a default listed in the specification.
 *
 *  default: PTHREAD_CREATE_JOINABLE
 *  Other valid values: PTHREAD_CREATE_DETACHED
   
 * 
 * Steps:
 * 1.  Create a thread using pthread_create() and passing 'NULL' for 'attr'.
 * 2.  Check to see if the thread is joinable, since that is the default.
 * 3.  We do this by calling pthread_join() and pthread_detach().  If 
 *     they fail, then the thread is not joinable, and the test fails.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

void *a_thread_func()
{
	
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	
	/* Create a new thread.  The default attribute should be that
	 * it is joinable. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* The new thread should be able to be joined. */
	if(pthread_join(new_th, NULL) == EINVAL)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
	/* The new thread should be able to be detached. */
	if(pthread_detach(new_th) == EINVAL)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;	
}


