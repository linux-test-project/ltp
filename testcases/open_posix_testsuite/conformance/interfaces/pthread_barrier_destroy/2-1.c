/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrier_destroy()
 *
 * The pthread_barrier_destroy( ) function may fail if:
 * [EBUSY] The implementation has detected an attempt to destroy a barrier while it is in
 * use (for example, while being used in a pthread_barrier_wait( ) call) by another
 * thread.
 *
 * Note: This case will always PASS
 *
 * Steps:
 * 1. Main initialize barrier with count 2
 * 2. Main create a child thread
 * 3. Child thread call pthread_barrier_wait(), should block
 * 4. Main call pthread_barrier_destroy(), while child is blocking on the barrier
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

static pthread_barrier_t barrier;
static int thread_state; 
#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* fn_chld(void *arg)
{ 
	int rc = 0;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	thread_state = ENTERED_THREAD;

	/* Child should block here */	
	printf("child: barrier wait\n");
	rc = pthread_barrier_wait(&barrier);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Error: child: pthread_barrier_wait() get unexpected "
			"return code : %d\n" , rc);
		exit(PTS_UNRESOLVED);
	} 
	else if(rc == PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("child: get PTHREAD_BARRIER_SERIAL_THREAD\n");
	}
	
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main()
{
	int cnt = 0;
	int rc;
	pthread_t child_thread;
	
	printf("main: Initialize barrier with count = 2\n");
	if(pthread_barrier_init(&barrier, NULL, 2) != 0)
	{
		printf("main: Error at pthread_barrier_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: create child thread\n");
	thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&child_thread, NULL, fn_chld, NULL) != 0)
	{
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Expect the child to block*/
	cnt = 0;
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 2); 
	
	if(thread_state == EXITING_THREAD)
	{
		/* child thread did not block */
		printf("Test FAILED: child thread did not block on "
			"pthread_barrier_wait()\n");
		exit(PTS_FAIL);
	}
	else if(thread_state != ENTERED_THREAD)
	{
		printf("Unexpected thread state\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: destroy barrier while child is waiting\n");

	rc = pthread_barrier_destroy(&barrier);
	
	if(rc == EBUSY)
	{
		printf("main: correctly got EBUSY\n");
		printf("Test PASSED\n");
	}
	else
	{
		printf("main: got return code: %d, %s\n", rc, strerror(rc));
		printf("Test PASSED: Note*: Expected EBUSY, but standard says 'may' fail.\n");
	}
	
	/* Cleanup (cancel thread in case it is still blocking */
	pthread_cancel(child_thread);
	
	return PTS_PASS;
}
