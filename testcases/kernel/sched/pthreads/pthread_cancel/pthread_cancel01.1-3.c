/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_cancel
 * Shall request that 'thread' shall be canceled.  The target thread's 
 * cancelability state and type determines when the cancelation takes
 * effect.
 * 
 * Test when a thread is PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_DEFERRED
 *  
 * STEPS:
 * 1. Create a thread
 * 2. In the thread function, set the state and type to
 *    PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_DEFERRED
 * 3. Send out a cancel request
 * 4. It should not be honored until a cancelation point is reached (which is
 *    test_cancel()).
 * 5. If it does cancel immediately, the cleanup handler will be called, and the
 *    test will fail.
 * 6. If it does not cancel at the cancelation point, then the operation will
 *    time out after 10 seconds and the test will fail.
 * 7. Otherwise, the test will pass.     
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */

int sem;		/* Manual semaphore */
int cleanup_flag;


void a_cleanup_func()	
{
	cleanup_flag=1;
	return;
}

void *a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	
	pthread_cleanup_push(a_cleanup_func,NULL);

	/* Tell main that the thread was created */
	sem=INMAIN;

	/* Wait for main to try and cancel thread, meaning until it
	 * sets sem=INTHREAD. Sleeping for 3 secs. to give time for the
	 * cancel request to be sent and processed. */
	while(sem==INMAIN)
		sleep(3);

	/* Should reach here if the thread correctly deffers the cancel
	 * request until a cancelation point has been reached (in this
	 * case, test_cancel()). */
	pthread_cleanup_pop(0);

	/* Cancelation point.  The thread should be canceled now. */
	pthread_testcancel();

	sleep(10);
	
	/* If we reach here, operation timed out */
	perror("Operation timed out, thread could not cancel itself\n");
	cleanup_flag=1;
	pthread_exit(PTHREAD_CANCELED);
}

int main()
{
	pthread_t new_th;
	int ret;
		
	/* Initializing values */
	sem=INTHREAD;
	cleanup_flag=0;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure thread is created before we cancel it. */
	while(sem==INTHREAD)
		sleep(1);

	/* Try and cancel thread.  It shouldn't cancel it. */
	if(pthread_cancel(new_th) != 0) 
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Indicate to the thread function that the thread cancel request
	 * has been sent to it. */
	sem=INTHREAD;

	/* Wait for thread to cancel itself with test_cancel */
	ret=pthread_join(new_th, NULL);

	if(ret != 0)
	{
		if(ret == ESRCH)
		{
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		
		perror("Error in pthread_join\n");
		return PTS_UNRESOLVED;		
	}

	if(cleanup_flag==1)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	

}


