/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_join()
 *  
 * When pthread_join() returns successfully, the target thread has been
 * terminated.
 * 
 * Steps:
 * 1.  Create a new thread.
 * 2.  Send a cancel request to it from main, then use pthread_join to wait for it to end.
 * 3.  The thread will sleep for 3 seconds, then call test_cancel() to cancel execution.
 * 4.  When this happens, the cleanup handler should be called.  
 * 5.  Main will test that when pthread_join allows main to continue with the process that
 *     the thread has ended execution.  If the cleanup_handler was not called, then the
 *     test fails.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define TIMEOUT 10	/* Timeout period for cancel request is 10 seconds. */
int cleanup_flag;	/* Flag to indicate the thread's cleanup handler was called */

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled. */
void a_cleanup_func()	
{
	cleanup_flag=1;
	return;
}

/* Thread's function. */
void *a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	
	/* Set up the cleanup handler */
	pthread_cleanup_push(a_cleanup_func,NULL);
	
	/* Wait for a timeout period for the cancel request to be sent. */
	/* Cancelation point.  Should call cleanup handler now. . */
	sleep(TIMEOUT);

	/* Should not get here, but just in case pthread_testcancel() didn't work. */
	/* Cancel request timed out. */
	pthread_cleanup_pop(0);
	cleanup_flag=-1;

	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;

	/* Initialize flag */
	cleanup_flag = 0;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Send cancel request to the thread.  */
	if(pthread_cancel(new_th) != 0) 
	{
		perror("Couldn't cancel thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to return */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if(cleanup_flag == 0)
	{
		printf("Test FAILED: Thread did not finish execution when pthread_join returned. \n");
		return PTS_FAIL;
	}	

	if(cleanup_flag == -1)
	{
		perror("Error in pthread_testcancel().  Cancel request timed out.\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
	
}


