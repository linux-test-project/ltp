/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrier_wait()
 *
 * If a signal is delivered to a thread blocked on a barrier..  
 * otherwise, the thread shall continue as normal from the completed barrier wait. Until
 * the thread in the signal handler returns from it, it is unspecified whether other threads may
 * proceed past the barrier once they have all reached it. 
 *
 * Steps:
 * 1. Main initialize barrier with count 2
 * 2. Main create a child thread
 * 3. Child thread call pthread_barrier_wait(), should block
 * 4. While child thread is blocking, send SIGUSR1 to child
 * 5. While child thread is in the signal handler, main thread call pthread_barrier_wait()
 * 6. After return from the signal handler, the child thread should continue as normal
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "posixtest.h"

static pthread_barrier_t barrier;
static int thread_state; 
static int sig_rcvd;
static int barrier_waited;

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

void sig_handler()
{
	struct timespec ts; 
	sig_rcvd = 1;
	printf("thread: interrupted by SIGUSR1\n");
	
	ts.tv_sec = 1;
	ts.tv_nsec = 0;
	while(barrier_waited != 1)
	{
		nanosleep(&ts, NULL);
	}
}

static void* fn_chld(void *arg)
{ 
	int rc = 0;
	struct sigaction act;	

	thread_state = ENTERED_THREAD;
	
	/* Set up thread to handle SIGUSR1 */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);
	
	printf("thread: call barrier wait\n");
	rc = pthread_barrier_wait(&barrier);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Test FAILED: child: pthread_barrier_wait() got unexpected "
			"return code : %d\n" , rc);
		exit(PTS_FAIL);
	} 
	else if(rc == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("thread: got PTHREAD_BARRIER_SERIAL_THREAD\n");
	
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

 
int main()
{
	int cnt = 0;
	int rc;
	pthread_t child_thread;
	sig_rcvd = 0;
	barrier_waited = 0;
	
	printf("Initialize barrier with count = 2\n");
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

	printf("main: send SIGUSR1 to child thread\n");
	if(pthread_kill(child_thread, SIGUSR1) != 0)
	{
		printf("main: Error at pthread_kill()\n");
		exit(PTS_UNRESOLVED);
	}

	/* Wait for thread to receive the signal */	
	while(sig_rcvd != 1)
	{
		sleep(1);
	}

	printf("main: call barrier wait\n");
	rc = pthread_barrier_wait(&barrier);
	
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Test FAILED: main: pthread_barrier_wait() got unexpected "
			"return code : %d\n" , rc);
		exit(PTS_FAIL);
	} 
	else if(rc == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("main: got PTHREAD_BARRIER_SERIAL_THREAD\n");
	
	barrier_waited = 1;
		
	/* We expected the child returned from barrier wait */
	cnt = 0;	
	do{
		sleep(1);
	}while (thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: child thread still blocked on "
			"barrier wait\n");
		return PTS_FAIL;
	}
	else if(thread_state != EXITING_THREAD)
	{
		printf("main: Unexpected thread state\n");
		return PTS_UNRESOLVED;
	}

	if(pthread_join(child_thread, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	if(pthread_barrier_destroy(&barrier) != 0)
	{
		printf("Error at pthread_barrier_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
