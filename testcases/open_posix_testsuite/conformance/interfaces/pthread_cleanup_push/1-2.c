/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * void pthread_cleanup_push(void (*routine) (void*), void *arg); 
 *
 * Shall push the specified cancelation cleanup handler routine onto the calling thread's
 * cancelation cleanup stack. The cancelation cleanup handler shall be popped from the 
 * cancelation cleanup stack and invoked with the argument arg when:
 *
 * (a)- The thread exits (calls pthread_exit())
 * (b)- The thread acts upon a cancelation request
 * (c)- the thread calls pthread_cleanup_pop() with a non-zero execution argument
 *  
 *  Testing (b)
 *  
 * STEPS:
 * 1. Create a thread
 * 2. In the thread, push a cancelation handler
 * 3. Main will cancel the thread before the thread exits
 * 4. Verify that the cleanup handler was called
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define CLEANUP_NOTCALLED 0 
# define CLEANUP_CALLED 1

# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */

int sem1;		/* Manual semaphore */
int cleanup_flag;

/* The cleanup handler */
void a_cleanup_func(void *flag_val)	
{
	cleanup_flag = (long)flag_val;
	sem1 = INMAIN;
	return;
}

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	pthread_cleanup_push(a_cleanup_func, (void*) CLEANUP_CALLED);
	
	/* Indicate to main() that the thread has been created. */	
	sem1=INMAIN;
	
	/* Wait until main() has sent out a cancel request, meaning until it
	 * sets sem1==INTHREAD */ 
	while(sem1==INMAIN)
		sleep(1);
	
	/* Give thread 10 seconds to time out.  If the cancel request was not
	 * honored until now, the test is unresolved because the cancel request
	 * was not handled correctly. */
	sleep(10);

	/* Shouldn't get here if the cancel request was honored immediately
	 * like it should have been. */
	pthread_cleanup_pop(0);
	pthread_exit((void*)PTS_UNRESOLVED);
	return NULL;
}

int main()
{
	pthread_t new_th;
	void *value_ptr;		/* hold return value of thread from pthread_join */ 

	/* Initializing values */
	sem1=INTHREAD;
	cleanup_flag=CLEANUP_NOTCALLED;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure thread is created before we cancel it. (wait for 
	 * a_thread_func() to set sem1=1.) */
	while(sem1==INTHREAD)
		sleep(1);

	if(pthread_cancel(new_th) != 0) 
	{
		printf("Error: Couldn't cancel thread\n");
		return PTS_UNRESOLVED;
	}

	/* Indicate to the thread function that the thread cancel request
	 * has been sent to it. */
	sem1=INTHREAD;
	
	/* Wait for thread to return. */
	if(pthread_join(new_th, &value_ptr) != 0)
	{
		printf("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure cancellation happened correctly */
	if((long)value_ptr == PTS_UNRESOLVED)
	{
		printf("Error: cancellation not correctly handled\n");
		return PTS_UNRESOLVED;
	}

	/* This means that the cleanup function wasn't called, so the cancel
	 * request was not honord immediately like it should have been. */
	if(cleanup_flag != CLEANUP_CALLED)
	{
		printf("Test FAILED: Cleanup hanlder not called up cancellation\n");
		return PTS_FAIL;
	}	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


