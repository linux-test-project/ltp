/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_create() creates a new thread with attributes specified
 * by 'attr', within a process.
 * 
 * Steps:
 * 1.  Create a thread using pthread_create()
 * 2.  Compare the thread ID of 'main' to the thread ID of the newly created
 *     thread. They should be different.
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

void *a_thread_func()
{
	
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t main_th, new_th;
	
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Obtain the thread ID of this main function */
	main_th=pthread_self();

	/* Compare the thread ID of the new thread to the main thread.
	 * They should be different.  If not, the test fails. */	
	if(pthread_equal(new_th, main_th) != 0)
	{
		printf("Test FAILED: A new thread wasn't created\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;	
}


