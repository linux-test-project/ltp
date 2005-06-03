/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_create() 
 * The thread is created executing 'start_routine' with 'arg' as its only
 * argument.
 *
 * Steps:
 * 1.  Create 5 separete threads using pthread_create() passing to it a single int 'arg'.
 * 2.  Use that passed int argument in the thread function start routine  and make sure no 
 *     errors occur.
 */

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

#define NUM_THREADS 5

/* The thread start routine. */
void *a_thread_func(void* num)
{
	intptr_t i = (intptr_t) num;
	printf("Passed argument for thread: %d\n", (int)i);

	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	long i;

	for(i=1;i<NUM_THREADS+1;i++)
	{	
		if(pthread_create(&new_th, NULL, a_thread_func, (void*)i) != 0)
		{	
			printf("Error creating thread\n");
			return PTS_FAIL;
		}
	
		/* Wait for thread to end execution */
		pthread_join(new_th, NULL);	
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


