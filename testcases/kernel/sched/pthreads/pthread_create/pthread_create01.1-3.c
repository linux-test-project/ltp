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
 * 1.  Create a new thread that will go into a never-ending while loop.
 * 2.  If the thread is truly asynchronise, then the main function will
 *     continue instead of waiting for the thread to return (which in never
 *     does in this test case).
 * 3.  An alarm is set to go off (i.e. send the SIGARLM signal) after 3
 *     seconds. This is done for 'timeing-out' reasons, in case main DOES
 *     wait for the thread to return.  This would also mean that the test
 *     failed.  
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

void *a_thread_function();
void alarm_handler();

pthread_t a;

int main()
{
	
	/* Set the action for SIGALRM to generate an error if it is
	 * reached. This is because if SIGALRM was sent, then the 
	 * test timed out. */
	if (signal(SIGALRM, alarm_handler) == SIG_ERR)
	{
		printf("Error in signal()\n");
		return PTS_UNRESOLVED;
	}

	/* SIGALRM will be sent in 5 seconds. */
	alarm(5);
	
	/* Create a new thread. */
	if(pthread_create(&a, NULL, a_thread_function, NULL) != 0)
	{
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	pthread_cancel(a);
	/* If 'main' has reached here, then the test passed because it means
	 * that the thread is truly asynchronise, and main isn't waiting for
	 * it to return in order to move on. */
	printf("Test PASSED\n");	
	return PTS_PASS;
}

/* A never-ending thread function */
void *a_thread_function()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	while(1)
		sleep(1);

	pthread_exit(0);
	return NULL;
}

/* If this handler is called, that means that the test has failed. */
void alarm_handler()
{
	printf("Test FAILED\n");
	exit(PTS_FAIL);
}

