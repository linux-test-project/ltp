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
 * 1.  Create a thread using pthread_create() passing to it an array of 'int's as an argument.
 * 2.  Use that passed int argument in the thread function start routine  and make sure no 
 *     errors occur.
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

#define NUM_THREADS 5

/* The thread start routine. */
void *a_thread_func(void* num)
{
	int *i, j;

	i = (int *)num;
		
	for(j=0;j<NUM_THREADS;j++)
		printf("Passed argument %d for thread\n", i[j]);

	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	int i[NUM_THREADS], j;

	for(j=0;j<NUM_THREADS;j++)
		i[j] = j+1;

	if(pthread_create(&new_th, NULL, a_thread_func, (void*)&i) != 0)
	{	
		printf("Error creating thread\n");
		return PTS_FAIL;
	}
	
	/* Wait for thread to end execution */
	pthread_join(new_th, NULL);	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


