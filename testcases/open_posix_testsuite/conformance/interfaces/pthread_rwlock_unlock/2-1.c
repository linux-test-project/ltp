/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
 *
 * pthread_rwlock_unlock( ) function shall release a lock held on the 
 * read-write lock object referenced by rwlock
 * If this function is called to release a write lock for this read-write 
 * lock object, the read-write lock object shall be put in the unlocked state.
 *
 * In the case: if a lock in an unlocked state, it can be acquired by a thread for write lock
 * successfully
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main thread write lock 'rwlock'
 * 3.  Create a child thread, the thread write lock 'rwlock', should block
 * 4.  Main thread unlock the write lock, the 'rwlock' is in unlocked state
 * 5.  Child thread should get the lock for writing.
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_rwlock_t rwlock;
static int thread_state; 

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
	int rc = 0;

	thread_state = ENTERED_THREAD;
	printf("thread: attempt write block\n");
	/* This should block */
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("thread: cannot get write lock\n");
		exit(PTS_UNRESOLVED);
	}
	printf("thread: acquired write lock\n");
	printf("thread: unlock write lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: thread failed to release write lock, Error Code=%d\n", rc);
		exit(PTS_FAIL);
	}
	thread_state = EXITING_THREAD;
	return NULL;
}
 
int main()
{
	int cnt = 0;
	int rc = 0;

	pthread_t wr_thread;
	
	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	/* This write lock should succeed */	
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	
	thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if(pthread_create(&wr_thread, NULL, fn_wr, NULL) != 0)
	{
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	cnt = 0;
	do{
		sleep(1);
	}while (thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state == EXITING_THREAD)
	{
		printf("Thread should block on write lock\n");
		exit(PTS_UNRESOLVED);
	}
	else if(thread_state != ENTERED_THREAD)
	{
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	/* thread_state == ENTERED_THREAD, i.e. thread correctly blocks on write lock */
	printf("main: unlock write lock\n");
	rc = pthread_rwlock_unlock(&rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: Main cannot release write lock\n");
		exit(PTS_FAIL);
	}

	cnt = 0;
	do{
		sleep(1);
	}while (thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(thread_state != EXITING_THREAD)
	{
		printf("Test FAILED: thread did not get write lock even when the lock has no owner\n");
		exit(PTS_FAIL);
	}

	if(pthread_join(wr_thread, NULL) != 0)
	{
		printf("Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
