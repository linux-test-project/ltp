/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test if the attributes specified by 'attr' are modified later, the thread's
 *  attributes shall not be affected.
 *  The attribute that will be tested is the detached state.
 *
 * 
 * Steps:
 * 1.  Set a pthread_attr_t object to be PTHREAD_CREATE_JOINABLE. 
 * 2.  Create a new thread using pthread_create() and passing this attribute
 *     object.   
 * 3.  Change the attribute object to be in a detached state rather than
 *     joinable.
 * 4.  Doing this should not effect the fact that the thread that was created
 *     is joinable, and so calling the functions pthread_detach() should not fail.   
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

# define TIMEOUT 10	/* Timeout value of 10 seconds. */
# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */

int sem1;		/* Manual semaphore */

void *a_thread_func()
{
	/* Indicate to main() that the thread was created. */
	sem1=INTHREAD;

	/* Wait for main to detach change the attribute object and try and detach this thread.
	 * Wait for a timeout value of 10 seconds before timing out if the thread was not able
	 * to be detached. */
	sleep(TIMEOUT);

	printf("Test FAILED: Did not detach the thread, main still waiting for it to end execution.\n");
	pthread_exit((void*)PTS_FAIL);
	return NULL;
}

int main()
{
	pthread_t new_th;
	pthread_attr_t new_attr;
	int ret;
	
	/* Initializing */
	sem1 = INMAIN;	
	if(pthread_attr_init(&new_attr) != 0)
	{
		perror("Error intializing attribute object\n");
		return PTS_UNRESOLVED;
		
	}

	/* Make the new attribute object joinable */
	if(pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_JOINABLE) != 0)
	{
		perror("Error setting the detached state of the attribute\n");
		return PTS_UNRESOLVED;
	}

	/* Create a new thread and pass it the attribute object that will
	 * make it joinable. */
	if(pthread_create(&new_th, &new_attr, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	while(sem1==INMAIN)
		sleep(1);
	
	/* Now change the attribute object to be in a detached state */
	if(pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED) != 0)
	{
		perror("Error setting the detached state of the attribute\n");
		return PTS_UNRESOLVED;
	}

	/* The new thread should still be able to be detached. */
	if((ret=pthread_detach(new_th)) == EINVAL)
	{
		printf("Test FAILED: pthread_detach failed on joinable thread. Return value is %d\n", ret);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


