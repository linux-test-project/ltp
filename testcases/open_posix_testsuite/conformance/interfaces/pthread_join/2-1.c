/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_join()
 *  
 * On return from a successful pthread_join() call with a non-NULL 'value_ptr'
 * argument, the avlue passed to pthread_exit() by the terminating thread shall
 * be made available in the location referenced by 'value_ptr'.
 *
 * Steps:
 * 1.  Create a new thread.  Have it return a return code on pthread_exit();
 * 2.  Call pthread_join() in main(), and pass to it 'value_ptr'.
 * 3.  Check to see of the value_ptr and the value returned by pthread_exit() are the same;
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define RETURN_CODE ((void*)100)	/* Set a random return code number. This shall be the return code of the
			 thread when using pthread_exit().*/

# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */

int sem;	/* Manual semaphore used to indicate when the thread has been created. */

/* Thread's function. */
void *a_thread_func()
{
	sem=INMAIN;
	pthread_exit(RETURN_CODE);
	return NULL;
}

int main()
{
	pthread_t new_th;
	void *value_ptr;

	/* Initializing variables. */
	value_ptr=0;
	sem=INTHREAD;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure the thread was created before we join it. */	
	while(sem==INTHREAD)
		sleep(1);

	/* Wait for thread to return */
	if(pthread_join(new_th, &value_ptr) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check to make sure that 'value_ptr' that was passed to pthread_join() and the
	 * pthread_exit() return code that was used in the thread funciton are the same. */
	if(value_ptr != RETURN_CODE)
	{
		printf("Test FAILED: pthread_join() did not return the pthread_exit value of the thread in 'value_ptr'.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}


