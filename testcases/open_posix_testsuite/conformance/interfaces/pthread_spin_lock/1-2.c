/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_spin_lock(pthread_spinlock_t *lock)
 *
 * The function shall lock the spin lock referenced by lock. The calling thread
 * shall acquire the lock if it is not held by another thread. Otherwise, the
 * thread shall spin (that is, shall not return from the pthread_spin_lock())
 * until the lock becomes available.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Create a child thread. The thread lock 'spinlock', should spin. 
 * 4.  Main thread unlock 'spinlock'
 * 5.  Child thread should get 'spinlock'
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
volatile static int thread_state; 

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* fn_chld(void *arg)
{ 
	int rc = 0;
	thread_state = ENTERED_THREAD;	

	/* Lock the spinlock */
	printf("thread: attempt spin lock\n");
	rc = pthread_spin_lock(&spinlock);
	if(rc != 0)
	{
		printf("Test FAILED: child failed to get spin lock,error code:%d\n" , rc);
		exit(PTS_FAIL);
	}
	printf("thread: acquired spin lock\n");
	
	/* Just some time between locking and unlocking */
	sleep(1);

	/* Unlock the spin lock */	
	printf("thread: unlock spin lock\n");
	if(pthread_spin_unlock(&spinlock))
	{
		printf("child: Error at pthread_spin_unlock()\n");
		exit(PTS_UNRESOLVED);
	}	
	
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int main()
{
	int cnt = 0;
	
	pthread_t child_thread;

	/* Initialize spinlock */	
	if(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0)
	{
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */	
	if(pthread_spin_lock(&spinlock) != 0)
	{
		printf("Test FAILED: main cannot get spin lock  when no one owns the lock\n");
		return PTS_FAIL;
	} 
	printf("main: acquired spin lock\n");

	/* Initialize thread state */	
	thread_state = NOT_CREATED_THREAD;

	/* Create thread */
	printf("main: create thread\n");
	if(pthread_create(&child_thread, NULL, fn_chld, NULL) != 0)
	{
		printf("main: Error creating child thread\n");
		return PTS_UNRESOLVED;
	}
	
	cnt = 0;
	/* Expect the child thread to spin on spin lock.  Wait for 3 seconds. */
	do{
		sleep(1);
	}while (thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == EXITING_THREAD)
	{
		printf("Test FAILED: child thread did not spin on spin lock when other thread holds the lock\n");
		return PTS_FAIL;
	}
	else if(thread_state != ENTERED_THREAD)
	{
		printf("main: Unexpected thread state %d\n", thread_state);
		return PTS_UNRESOLVED;
	}
	
	printf("main: unlock spin lock\n");
	if(pthread_spin_unlock(&spinlock) != 0)
	{
		printf("main: Error at pthread_spin_unlock()\n");
		return PTS_UNRESOLVED;
	}
	
	/* We expected the child get the spin lock and exit */
	cnt = 0;	
	do{
		sleep(1);
	}while (thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: child thread did not get spin lock\n");
		return PTS_FAIL;
	}
	else if(thread_state != EXITING_THREAD)
	{
		printf("main: Unexpected thread state %d\n", thread_state);
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to finish execution */	
	if(pthread_join(child_thread, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the spinlock */
	if(pthread_spin_destroy(&spinlock) != 0)
	{
		printf("Error at pthread_spin_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
