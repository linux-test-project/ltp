/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 *  void pthread_cleanup_push(void (*routine) (void*), void *arg); 
 *
 * Shall push the specified cancelation cleanup handler routine onto the calling thread's
 * cancelation cleanup stack. The cancelation cleanup handler shall be popped from the 
 * cancelation cleanup stack and invoked with the argument arg when:
 *
 * (a)- The thread exits (calls pthread_exit())
 * (b)- The thread acts upon a cancelation request
 * (c)- the thread calls pthread_cleanup_pop() with a non-zero execution argument
 *  
 *  Testing (c)
 *  
 * STEPS:
 * 1. Create a thread
 * 2. The thread will push a cleanup handler routine, then pop the cleanup handler routine.
 * 3. Verify that the cleanup handler was called.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define CLEANUP_NOTCALLED 0 
# define CLEANUP_CALLED 1

int cleanup_flag;

/* Cleanup handler */
void a_cleanup_func(void *flag_val)	
{
	cleanup_flag = (long)flag_val;
	return;
}

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	pthread_cleanup_push(a_cleanup_func, (void*) CLEANUP_CALLED);
	pthread_cleanup_pop(1);
	
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;

	/* Initializing values */
	cleanup_flag = CLEANUP_NOTCALLED;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end execution */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Check to verify that the cleanup handler was called */
	if(cleanup_flag != CLEANUP_CALLED)
	{
		printf("Test FAILED: Cleanup handler not called\n");
		return PTS_FAIL;
	}	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


