/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_exit()
 *  
 * Any cancelation cleanup handlers that have been pushed and not yet popped
 * shall be popped and executed in the reverse order that they were pushed.
 * 
 * Steps:
 * 1.  Create a new thread.
 * 2.  The thread will call pthread_exit().
 * 3.  When this happens, the cleanup handler should be called.  
 * 4.  Main will test that when pthread_join allows main to continue with the process that
 *     the thread has ended execution.  If the cleanup_handler was not called, then the
 *     test fails.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

int i[3], j;

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled. */
void a_cleanup_func1()	
{
	i[j]=1;
	j++;
	return;
}

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled. */
void a_cleanup_func2()	
{
	i[j]=2;
	j++;
	return;
}

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled. */
void a_cleanup_func3()	
{
	i[j]=3;
	j++;
	return;
}
/* Thread's function. */
void *a_thread_func()
{
	/* Set up 3 cleanup handlers */
	pthread_cleanup_push(a_cleanup_func1,NULL);
	pthread_cleanup_push(a_cleanup_func2,NULL);
	pthread_cleanup_push(a_cleanup_func3,NULL);
	
	/* Terminate the thread here. */
	pthread_exit(0);
	
	/* Need these here for it to compile nicely.  We never reach here though. */
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	
	/* Initialize integer array. */
	for(j=0;j<3;j++)
		i[j] = 0;

	/* Initialize counter. */
	j=0;

	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to return */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check to make sure that the cleanup handlers were executed in order. */
	if(i[0] == 3)
	{
		if(i[1] == 2)
		{
			if(i[2] == 1)
			{
				printf("Test PASSED\n");
				return PTS_PASS;
				
			}
			printf("Test FAILED: Did not execute cleanup handlers in order.\n");
			return PTS_FAIL;
		}
		printf("Test FAILED: Did not execute cleanup handlers in order.\n");
		return PTS_FAIL;
	}
	else if(i[0] == 0)
	{
		printf("Test FAILED: Did not execute cleanup handlers.\n");
		return PTS_FAIL;
	}
	
	printf("Test FAILED: Did not execute cleanup handlers in order.\n");
	return PTS_FAIL;
	
}


