/*   
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_cancel
 * When the cancelation is acted on, the cancelation cleanup handlers for
 * 'thread' shall be called "asynchronously"
 *
 * STEPS:
 * 1. Change main thread to a real-time thread with a high priority
 * 1. Create a lower priority thread
 * 2. In the thread function, push a cleanup function onto the stack
 * 3. Cancel the thread from main and get timestamp, then block.
 * 4. The cleanup function should be automatically 
 *    executed, else the test will fail.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"
#include <time.h>

#define TEST "3-1"
#define FUNCTION "pthread_cancel"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define FIFOPOLICY SCHED_FIFO
#define MAIN_PRIORITY 30
#define TIMEOUT_IN_SECS 10

/* Manual semaphore */
int sem;			

/* Made global so that the cleanup function
 * can manipulate the value as well. 
 */
int cleanup_flag;		
struct timespec main_time, cleanup_time;

/* A cleanup function that sets the cleanup_flag to 1, meaning that the
 * cleanup function was reached. 
 */ 
void a_cleanup_func()	
{
	clock_gettime(CLOCK_REALTIME, &cleanup_time);
	cleanup_flag = 1;
	sem = 0;
	return;
}

/* A thread function called at the creation of the thread. It will push
 * the cleanup function onto it's stack, then go into a continuous 'while'
 * loop, never reaching the cleanup_pop function.  So the only way the cleanup
 * function can be called is when the thread is canceled and all the cleanup
 * functions are supposed to be popped. 
 */
void *a_thread_func()
{
	int rc = 0;

	/* To enable thread immediate cancelation, since the default
	 * is PTHREAD_CANCEL_DEFERRED. 
	 */
	rc = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_setcanceltype\n");
		exit(PTS_UNRESOLVED);
	}
	pthread_cleanup_push(a_cleanup_func, NULL);

	sem=1;
	while(sem == 1)
		sleep(1);
	sleep(5);
	sem=0;

	/* Should never be reached, but is required to be in the code
	 * since pthread_cleanup_push is in the code.  Else a compile error
	 * will result.
	 */
	pthread_cleanup_pop(0);
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t                  new_th;
	int                        i;
	double                      diff ;
	struct sched_param         param;
	int                        rc = 0;

	/* Initializing the cleanup flag. */
	cleanup_flag = 0;
	sem = 0;
	param.sched_priority = MAIN_PRIORITY;

	/* Increase priority of main, so the new thread doesn't get to run */
	rc = pthread_setschedparam(pthread_self(), FIFOPOLICY, &param);
	if(rc != 0) {	
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		exit(PTS_UNRESOLVED);
	}
	
	/* Create a new thread. */
	rc = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if(rc != 0) {	
		printf(ERROR_PREFIX "pthread_create\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure thread is created and executed before we cancel it. */
	while(sem == 0)
		sleep(1);

	rc = pthread_cancel(new_th);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_cancel\n");
		exit(PTS_FAIL);
	}

	/* Get the time after canceling the thread */
	clock_gettime(CLOCK_REALTIME, &main_time);
	i = 0;
	while(sem == 1) {
		sleep(1);	
		if(i == TIMEOUT_IN_SECS) {
			printf(ERROR_PREFIX "Cleanup handler was not called\n");
			exit(PTS_FAIL);
		}
		i++;
	}

	/* If the cleanup function was not reached by calling the
	 * pthread_cancel function, then the test fails. 
	 */
	if(cleanup_flag != 1) {
		printf(ERROR_PREFIX "Cleanup handler was not called\n");
		exit(PTS_FAIL);
	}
	
	diff = cleanup_time.tv_sec - main_time.tv_sec;
	diff += (double)(cleanup_time.tv_nsec - main_time.tv_nsec)/1000000000.0;
	if(diff < 0) {
		printf(ERROR_PREFIX "Cleanup function was called before main continued\n");
		exit(PTS_FAIL);
	}
	printf("Test PASS\n");
	exit(PTS_PASS);	
}
