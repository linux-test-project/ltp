/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_create().
 * 
 * If pthread_create() fails, no new thread is created and the contents of the
 * location referenced by 'thread' is undefined.
 *
 * EXPLANATION:  3 ways that pthread_create can fail is if (1) The system lacked memory
 * resources.  (2) The caller did not have the appropriate permissions. (3) An invalid
 * attributes object was passed.  Since the last situation is the easiest to implement, that
 * is the one I chose to use in order to make pthread_create fail.
 *
 * The problem with that is that accessing an uninitialized attributes object will cause a 
 * segmentation fault (since it usually points to garbage).  So the idea here is to catch
 * the SIGSEGV signal (segmentation faults), and to see if the thread was ever created, by
 * setting a flag in the thread's starting routine.
 * 
 * I recognize that causing pthread_create() to fail gracefully is a very difficult task,
 * especially taking into consideration that a lot of it is implementation-specific.  I did
 * try to manually set the members of a pthread_attr_t object to invalid values, but that didn't
 * seem to make pthread_create() fail at all, in any of the implementations I tested on. So for
 * now, this is what we have.
 * 
 * 
 * Steps:
 * 1.  Create a thread using pthread_create() with an invalid attributes object (uninitialized).
 * 2.  Catch the SIGSEGV (seg fault) signal.  In the signal handler, check to make sure that
 *     the start routine for the thread was never reached, meaning the thread was never created.
 * 3.  If SIGSEGV was not caught in 10 seconds, the test times out and fails.  If the signal
 *     was caught, but the thread start routine was called at some point, the test also fails.
 *
 *
 * - adam.li@intel.com: 2004-04-30
 * This case will end with segmentation fault. 
 * I happened to find on NPTL pthread_create() can fail is 
 * pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_INHERIT), while
 * does not call pthread_attr_setschedparam() to set the prority. (since
 * by default the priority will be 0. But this is implementation specific.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

int created_thread;	/* Flag indicating that the thread start routine was reached, and
			   therefore, the thread was created. */
int segfault_flag; 	/* Flag indicating that a segmentation fault occured. */

/* Thread's start routine. */
void *a_thread_func()
{
	/* Indicate that the thread start routine was reached.  If it was reached, the test
	 * fails, as the thread should have not been created in the first place. */
	created_thread = 1;
	pthread_exit(0);
	return NULL;
}

/* Signal handler for SIGSEGV (segmentation fault signal) */
void sig_handler(int sig)
{
	/* If a segmentation fault occured when it was supposed to (i.e. when pthread_create()
	 * was called with the invalid attributes object). */
	if(segfault_flag == 1)
	{
		/* check if the thread start routine was called. If yes, then the thread was
		 * created, meaning the test fails. */
		if(created_thread == 1)
		{
			printf("Test FAILED: Created thread though an invalid attribute was passed to pthread_create().\n");
			pthread_exit((void*)PTS_FAIL);
		}

		printf("Test PASSED\n");
		pthread_exit((void*)PTS_PASS);
		return;
	}

	printf("Test FAILED: Did not receive segmentation fault signal, waited 10 seconds.\n");
	pthread_exit((void*)PTS_FAIL);
	return;
}

/* MAIN */
int main()
{
	pthread_t new_th;
	pthread_attr_t inv_attr;
	struct sigaction act;

	/* Inializing flags. */
	segfault_flag = 1;
	created_thread = 0;

	/* Set signal handler for SIGSEGV (seg fault) */
	act.sa_handler = sig_handler;
	act.sa_flags = 0;
	sigaction(SIGSEGV, &act, NULL);
	
	/* Create the thread with the invalid, uninitialized attributes object. */
	pthread_create(&new_th, &inv_attr, a_thread_func, NULL);

	/* Should not reach here if process correctly seg-faulted. */
	segfault_flag = 0;

	/* Timeout after 10 seconds if a segfault was not encountered.  The test then fails. */
	sleep(10);	

	/* Manually send the SIGSEGV signal to the signal handler.  If this point is reached, 
	 * the test fails. */
	if(raise(SIGSEGV) != 0)
	{
		perror("Error in raise()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test FAILED.\n");
	return PTS_FAIL;
}


