/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * 
 * Test pthread_once()
 *
 * The pthread_once() function is not a cancelation point.  However if
 * 'init_routine'is a cancelation point and is canceled, the effect on 
 * 'once_control' shall be as if pthread_once() was never called.
 *   
 * STEPS:
 * 1. Create a cancelable thread.
 * 2. In the thread routine, call pthread_once using a global pthread_once_t
 *    object.
 * 3. Cancel the thread during the pthread_once init function
 * 4. Call pthread_once again with the same pthread once_t object
 * 5. This should call the pthread_once init function.  If not, the test fails.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

/* Global pthread_once_t object */
pthread_once_t once_control;

/* Keeps track of how many times the init function has been called. */
int init_flag;

/* The init function that pthread_once calls */
void *an_init_func()
{
	/* Indicate to main() that the init function has been reached */
	init_flag=1;
	
	/* Stay in a continuous loop until the thread that called
	 * this function gets canceled */ 
	sleep(10);

	/* The thread could not be canceled, timeout after 10 secs */
	perror("Init function timed out (10 secs), thread could not be canceled\n");
	init_flag=-1;
	return NULL;
}

/* Thread function */
void *a_thread_func()
{
	/* Make the thread cancelable immediately */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	pthread_once(&once_control, (void*)an_init_func);
	return NULL;
}

/* 2nd init function used by the 2nd call of pthread_once */
void *an_init_func2()
{
	/* Indicate to main() that this init function has been reached */
	init_flag=1;
	return NULL;
}

int main()
{
	pthread_t new_th;
	once_control = PTHREAD_ONCE_INIT;
	init_flag=0;
	
	/* Create a thread that will execute the first call to pthread_once() */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait until the init function is reached to cancel the thread */
	while(init_flag==0)
		sleep(1);	

	/* Send cancel request to the thread*/
	if(pthread_cancel(new_th) != 0) 
	{
		perror("Could send cancel request to thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait until the thread is canceled */	
	pthread_join(new_th, NULL);

	/* If the thread could not be canceled and timed out, send
	 * an error */ 	
	if (init_flag == -1)
	{
		perror("Error: could not cancel thread\n");
		return PTS_UNRESOLVED;
	}
	
	init_flag=0;

	/* Should be able to call pthread_once() again with the same
	 * pthread_once_t object. */
	pthread_once(&once_control, (void*)an_init_func2);

	/* If the init function from the 2nd call to pthread_once() was not
 	 * reached, the test fails. */	
	if(init_flag != 1)
	{
		printf("Test FAILED\n: %d", init_flag);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}


