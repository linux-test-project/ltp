/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_cancel
 * When the cancelation is acted on, the cancelation cleanup handlers for
 * 'thread' shall be called.
 *
 * STEPS:
 * 1. Create a thread
 * 2. In the thread function, push a cleanup function onto the stack
 * 3. Cancel the thread.  The cleanup function should be automatically 
 *    executed, else the test will fail.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int sem;			/* Manual semaphore */
int cleanup_flag;		/* Made global so that the cleanup function
				   can manipulate the value as well. */

/* A cleanup function that sets the cleanup_flag to 1, meaning that the
 * cleanup function was reached. */ 
void a_cleanup_func()	
{
	cleanup_flag=1;
	sem=0;
	return;
}

/* A thread function called at the creation of the thread. It will push
 * the cleanup function onto it's stack, then go into a continuous 'while'
 * loop, never reaching the cleanup_pop function.  So the only way the cleanup
 * function can be called is when the thread is canceled and all the cleanup
 * functions are supposed to be popped. */
void *a_thread_func()
{
	/* To enable thread immediate cancelation, since the default
	 * is PTHREAD_CANCEL_DEFERRED. */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_cleanup_push(a_cleanup_func,NULL);
	sem=1;
	while(sem==1)
		sleep(1);
	sleep(5);
	sem=0;
	/* Should never be reached, but is required to be in the code
	 * since pthread_cleanup_push is in the code.  Else a compile error
	 * will result. */
	pthread_cleanup_pop(0);
	perror("Operation timed out, thread could not be canceled\n");
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	int i;
	/* Initializing the cleanup flag. */
	cleanup_flag=0;
	sem=0;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure thread is created before we cancel it. */
	while(sem==0)
		sleep(1);

	if(pthread_cancel(new_th) != 0)
	{
		printf("Error canceling thread\n");
		return PTS_FAIL;
	}

	i=0;
	while(sem==1)
	{
		sleep(1);	
		if(i==10)
		{
			printf("Test FAILED: Timed out while waiting for cancelation cleanup handlers to execute\n");
			return PTS_FAIL;
		}
		i++;
	}

	/* If the cleanup function was not reached by calling the
	 * pthread_cancel function, then the test fails. */
	if(cleanup_flag != 1)
	{
		printf("Test FAILED: Could not cancel thread\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;	
}


