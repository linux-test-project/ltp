/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
 *
 * The function shall apply a write lock like the pthread_rwlock_wrlock(), with the exception 
 * that the funciton shall fail if any thread currently holds rwlock(for reading and writing).
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'rwlock' for reading with pthread_rwlock_rdlock()
 * 3.  Create a child thread, the thread locks 'rwlock' for writing, using 
 *     pthread_rwlock_trywrlock(), should get EBUSY
 * 4.  Main thread unlocks 'rwlock'
 * 5.  Main thread locks 'rwlock' for writing, using pthread_rwlock_trywrlock(), 
 *     should get the lock successfully
 * 6.  Create child thread to lock 'rwlock' for writing, with pthread_rwlock_trywrlock(), 
 *     should get EBUSY
 * 7.  Main thread unlock 'rwlock'
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int thread_state;
static int get_ebusy; 

/* thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* fn_wr(void *arg)
{ 
	thread_state = ENTERED_THREAD;
	int rc;

	printf("thread: attempt pthread_rwlock_trywrlock()\n");	
	rc = pthread_rwlock_trywrlock(&rwlock);
	if(rc != EBUSY)
	{
		printf("Test FAILED: thread: Expected EBUSY, got %d\n", rc);
		exit(PTS_FAIL);
	}
	get_ebusy = 1;
	printf("thread: correctly got EBUSY\n");
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int main()
{
	int cnt = 0;
	int rc = 0;
	pthread_t thread1, thread2;
	
	get_ebusy = 0;
	
	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: acquired read lock\n");
	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread1\n");
	if(pthread_create(&thread1, NULL, fn_wr, NULL) != 0)
	{
		printf("Error creating thread1\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	/* We do no expect thread1 to block */
	cnt = 0;
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == ENTERED_THREAD)
	{
		/* the child thread blocked */
		printf("Test FAILED: thread1 should not block on pthread_rwlock_trywrlock()\n");
		exit(PTS_FAIL);
	}
	else if(thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state for thread1: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}
		
	if(get_ebusy != 1)
	{
		printf("Test FAILED: thread1 should get EBUSY\n");
		exit(PTS_FAIL);
	}

	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("main: Error releasing read lock\n");
		exit(PTS_UNRESOLVED);
	}
		
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("main: Error joining thread1\n");
		exit(PTS_UNRESOLVED);
	}
	
	printf("main: attempt pthread_rwlock_trywrlock()\n");
	/* Should get the write lock*/
	rc = pthread_rwlock_trywrlock(&rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: main failed at pthread_rwlock_trywrlock(), error code: %d\n", rc);
		return PTS_FAIL;	
	}

	printf("main: acquired write lock\n");

	get_ebusy = 0;
	thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create thread2\n");
	if(pthread_create(&thread2, NULL, fn_wr, NULL) != 0)
	{
		printf("main: Error creating thread2\n");
		return PTS_UNRESOLVED;
	}
	
	/* We do not expect thread2 to block */
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread2 should not block on pthread_rwlock_trywrlock()\n");
		exit(PTS_FAIL);
	}
	else if(thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	if(get_ebusy != 1)
	{
		printf("Test FAILED: thread2 should get EBUSY\n");
		exit(PTS_FAIL);
	}
	
	printf("main: unlock write lock\n");
	thread_state = 1;
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("main: Error at 2nd pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}

	if(pthread_join(thread2, NULL) != 0)
	{
		printf("main: Error at 2cn pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
