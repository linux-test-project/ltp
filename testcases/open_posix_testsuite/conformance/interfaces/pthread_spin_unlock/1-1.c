/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test pthread_spin_unlock(pthread_spinlock_t *lock)
 *
 * The function shall release the spin lock referenced by 'lock' which
 * was locked via the pthread_spin_lock() or pthread_spin_trylock().
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock' with pthread_spin_lock(),  should get the lock
 * 3.  Main thread unlock 'spinlock'
 * 4.  Create a child thread. The thread try to lock 'spinlock', with
 *     pthread_spin_trylock(), should get the lock.
 * 5.  Child thread unlock
 * 6.  Main thread lock 'spinlock', using pthread_spin_trylock(), should get
 *     lock 
 * 7.  Main thread unlock 'spinlock'
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

	printf("thread: attempt trylock\n");
	rc = pthread_spin_trylock(&spinlock);
	if(rc != 0)
	{
		printf("Test FAILED: thread failed to get spin lock," 
			"Error code:%d\n" , rc);
		exit(PTS_FAIL);
	}
	printf("thread: acquired spin lock\n");
	
	printf("thread: unlock spin lock\n");
	if(pthread_spin_unlock(&spinlock))
	{
		printf("thread: Error at pthread_spin_unlock()\n");
		exit(PTS_FAIL);
	}	
	
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int main()
{
	int rc;
	
	pthread_t child_thread;
	
	if(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0)
	{
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */	
	if(pthread_spin_lock(&spinlock) != 0)
	{
		printf("main: cannot get spin lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	} 
	printf("main: acquired spin lock\n");

	printf("main: unlock spin lock\n");
	rc = pthread_spin_unlock(&spinlock);
	if(rc != 0)
	{
		printf("main: Error at pthread_spin_unlock()\n");
		return PTS_FAIL;
	}

	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if(pthread_create(&child_thread, NULL, fn_chld, NULL) != 0)
	{
		printf("main: Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to end execution */
	if(pthread_join(child_thread, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: try to lock again when thread unlocked\n");
	if(pthread_spin_trylock(&spinlock) != 0)
	{
		printf("main: Should get spin lock\n");
		return PTS_FAIL;
	}
	
	printf("main: acquired spin lock\n");
	printf("main: unlock spin lock\n");
	if(pthread_spin_unlock(&spinlock) != 0)
	{
		printf("Test FAILED: main: Error at pthread_spin_unlock()\n");
		return PTS_FAIL;
	}	

	if(pthread_spin_destroy(&spinlock) != 0)
	{
		printf("Error at pthread_spin_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
