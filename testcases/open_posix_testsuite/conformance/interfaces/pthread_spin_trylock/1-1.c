/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_spin_trylock(pthread_spinlock_t *lock)
 *
 * The function shall lock the spin lock referenced by 'lock' if it is not
 * held by any thread. Otherwise, the function shall fail. 
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', using pthread_spin_trylock(), 
 *     should get the lock successfully.
 * 3.  Create a child thread. The thread lock 'spinlock', 
 *     using pthread_spin_trylock(), shall fail with EBUSY.
 * 4.  If the child spins on the lock, after 2 seconds, send SIGALRM to it.
 * 5.  Child thread check its state in the signal handler.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
volatile static int thread_state; 
int rc; 

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void sig_handler()
{
	if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread incorrectly spins on trylock\n");
		exit(PTS_FAIL);
	}
	else
	{
		printf("UNRESOLVED: Unexpected child thread state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}
}

static void* fn_chld(void *arg)
{ 
	rc = 0;
	
	struct sigaction act;
	thread_state = ENTERED_THREAD;

	/* Set up child thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	
	/* thread: send SIGALRM to me after 2 seconds in case in spins on trylock */
	alarm(2);

	printf("thread: attempt trylock\n");
	rc = pthread_spin_trylock(&spinlock);

	pthread_exit(0);
	return NULL;
}
 
int main()
{
	pthread_t child_thread;
	
	if(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0)
	{
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt to trylock\n");

	/* We should get the lock */	
	if(pthread_spin_trylock(&spinlock) != 0)
	{
		printf("Test FAILED: main cannot get spin lock when no one owns the lock\n");
		return PTS_FAIL;
	} 
	printf("main: acquired spin lock\n");
	
	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if(pthread_create(&child_thread, NULL, fn_chld, NULL) != 0)
	{
		printf("main: Error creating child thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to end execution */
	pthread_join(child_thread, NULL);

	/* Check the return code of pthread_spin_trylock */
	if(rc != EBUSY)
	{
		printf("Test FAILED: pthread_spin_trylock should return EBUSY, instead got error code:%d\n" , rc);
		return PTS_FAIL;
	} 
	
	printf("thread: correctly returned EBUSY on trylock\n");
	printf("Test PASSED\n");
	return PTS_PASS;
	
}
