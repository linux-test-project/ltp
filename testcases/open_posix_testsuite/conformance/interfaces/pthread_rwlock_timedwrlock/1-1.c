/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock)
 *
 *	pthread_rwlock_timedwrlock( ) function shall apply a write lock to the 
 *	read-write lock referenced by rwlock as in the pthread_rwlock_wrlock( ) function. 
 *	However, if the lock cannot be acquired without waiting for other threads to 
 *	unlock the lock, this wait shall be terminate when the specified timeout expires. 
 *
 * Steps:
 * 1.  Initialize rwlock
 * 2.  Main creats thread0. 
 * 3.  Thread0 does pthread_rwlock_timedwrlock(), should get the lock successfully then unlock.
 * 4.  Main thread locks 'rwlock' for reading with pthread_rwlock_rdlock()
 * 5.  Create a child thread, the thread time-locks 'rwlock' for writing, 
 *	using pthread_rwlock_timedwrlock(), should block, but when the timer expires, 
 *	that wait will be terminated.
 * 6.  Main thread unlocks 'rwlock'
 * 7.  Main thread locks 'rwlock' for writing.
 * 8.  Create child thread to lock 'rwlock' for writing, using pthread_rwlock_timedwrlock,
 *	 it should block but when the timer expires, the wait will be terminated
 * 8.  Main thread unlocks 'rwlock'
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

#define TIMEOUT 3

static pthread_rwlock_t rwlock;
static int thread_state; 
static struct timeval currsec1, currsec2;
static int expired;

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
	struct timespec timeout;
	int rc;
	thread_state = ENTERED_THREAD;

	gettimeofday(&currsec1, NULL);

	/* Absolute time, not relative. */
	timeout.tv_sec = currsec1.tv_sec + TIMEOUT;
	timeout.tv_nsec = currsec1.tv_usec * 1000;	
	
	printf("thread: attempt timed write lock, %d secs\n", TIMEOUT);	
	rc = pthread_rwlock_timedwrlock(&rwlock, &timeout);
	if(rc  == ETIMEDOUT)
	{
		printf("thread: timer expired\n");
		expired = 1;
	}
	else if(rc == 0)
	{
		printf("thread: acquired write lock\n");
		expired = 0;
		printf("thread: unlock write lock\n");
		if(pthread_rwlock_unlock(&rwlock) != 0)
		{
			printf("thread: error release write lock\n");
			exit(PTS_UNRESOLVED);
		}
	}
	else
	{
		printf("thread: Error in pthread_rwlock_timedrdlock().\n");
		exit(PTS_UNRESOLVED);
	}
	
	/* Get time after the mutex timed out in locking. */
	gettimeofday(&currsec2, NULL);
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int main()
{
	int cnt = 0;
	pthread_t thread0, thread1, thread2;
	
	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: create thread0\n");
	thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&thread0, NULL, fn_wr, NULL) != 0)
	{
		printf("Error creating thread0\n");
		return PTS_UNRESOLVED;
	}
	
	/* thread0 should not block at all since no one else has locked rwlock */

	cnt = 0;
	expired = 0;
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
	
	if(thread_state == EXITING_THREAD)
	{
		if(expired == 1)
		{
			printf("Test FAILED: the timer expired\n");
			exit(PTS_FAIL);
		}
		else
			printf("thread0 correctly acquired the write lock.\n");
	}
	else if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread0 incorrectly blocked on timedwrlock\n");
		exit(PTS_FAIL);
	}
	else
	{
		printf("Unexpected state for thread0 %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}	
	
	if(pthread_join(thread0, NULL) != 0)
	{
		printf("Error when joining thread0\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt read lock\n");
	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired read lock\n");
	
	thread_state = NOT_CREATED_THREAD;
	
	printf("main: create thread1\n");
	if(pthread_create(&thread1, NULL, fn_wr, NULL) != 0)
	{
		printf("Error when creating thread1\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after TIMEOUT*2 seconds,
	   we regard it as blocked */

	/* we expect the thread to expire blocking after timeout*/
	cnt = 0;
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
		
	if(thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = currsec2.tv_sec - currsec1.tv_sec;
		time_diff.tv_usec = currsec2.tv_usec - currsec1.tv_usec;
		if (time_diff.tv_usec < 0)
		{
			--time_diff.tv_sec;
			time_diff.tv_usec += 1000000;
		}
		if(time_diff.tv_sec < TIMEOUT)
		{
			printf("Test FAILED: the timer expired and blocking was terminated, but the timeout is not correct: expected to wait for %d, but waited for %ld.%06ld\n", TIMEOUT, (long)time_diff.tv_sec, (long)time_diff.tv_usec);
			exit(PTS_FAIL);
		}
		else
			printf("thread1 correctly expired at timeout.\n");
	}
	else if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: wait is not terminated even when the timer expired\n");
		exit(PTS_FAIL);
	}
	else
	{
		printf("Unexpected state for thread1 %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}
	
	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("Error when release read lock\n");
		exit(PTS_UNRESOLVED);
	}
	
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("Error when joining thread1\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;	
	}
	printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create thread2\n");
	if(pthread_create(&thread2, NULL, fn_wr, NULL) != 0)
	{
		printf("Error when creating thread2\n");
		return PTS_UNRESOLVED;
	}
	
	/* we expect thread2 to expire blocking after timeout */
	do{
		sleep(1);
	}while (thread_state !=EXITING_THREAD && cnt++ < 2*TIMEOUT); 
	
	if(thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = currsec2.tv_sec - currsec1.tv_sec;
		time_diff.tv_usec = currsec2.tv_usec - currsec1.tv_usec;
		if (time_diff.tv_usec < 0)
		{
			--time_diff.tv_sec;
			time_diff.tv_usec += 1000000;
		}
		if(time_diff.tv_sec < TIMEOUT)
		{
			printf("Test FAILED: for thread 2, the timer expired and waiter terminated, but the timeout is not correct\n");
			exit(PTS_FAIL);
		}
		else
			printf("thread2 correctly expired at timeout.\n");
		
	}
	else if(thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: for thread2, wait is not terminated even when the timer expired\n");
		exit(PTS_FAIL);
	}
	else
	{
		printf("Unexpected state for thread2 %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock write lock\n");
	thread_state = NOT_CREATED_THREAD;
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("Error releasing write lock\n");
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
