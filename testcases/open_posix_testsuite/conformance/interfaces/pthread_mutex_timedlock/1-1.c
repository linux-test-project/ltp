/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_timedlock()
 * locks the mutex object referenced by 'mutex'.  If the mutex is
 * already locked, the calling thread shall block until the mutex becomes
 * available.  The wait will end when the specified timeout time has expired.

 * The timeout expires when the absolute time 'abs_timeout' passes, or if 'abs_timeout'
 * has already been passed the time of the call.

 * Steps: 
 *
 * 1. Create a mutex in the main() thread and lock it.
 * 2. Create a thread, and call pthread_mutex_timedlock inside of it.  It should block for
 *    the set time of (3 secs.).
 * 3. Save the time before and after the thread tried to lock the mutex.
 * 4. After the thread has ended, main() will compare the times before and after the mutex
 *    tried to lock in the thread.
 */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT 3					/* 3 seconds of timeout time for
							   pthread_mutex_timedlock(). */
void *f1(void *parm);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	/* The mutex */
time_t currsec1, currsec2;				/* Variables for saving time before 
						           and afer locking the mutex using
							   pthread_mutex_timedlock(). */	   
/****************************
 *
 * MAIN()
 *
 * *************************/
int main()
{
	pthread_t new_th;

	/* Lock the mutex. */
	if(pthread_mutex_lock(&mutex) != 0)
	{
		perror("Error in pthread_mutex_lock().\n");
		return PTS_UNRESOLVED;
	}

	/* Create a thread that will call pthread_mutex_timedlock */	
	if(pthread_create(&new_th, NULL, f1, NULL) != 0)
	{
		perror("Error in pthread_create().\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end. */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join().\n");
		return PTS_UNRESOLVED;
	}

	/* Cleaning up the mutexes. */
	if(pthread_mutex_unlock(&mutex) != 0)
	{
		perror("Error in pthread_mutex_unlock().\n");
		return PTS_UNRESOLVED;
	}
	if(pthread_mutex_destroy(&mutex) != 0)
	{
		perror("Error in pthread_mutex_destroy().\n");
		return PTS_UNRESOLVED;
	}

	/* Compare time before the mutex locked and after the mutex lock timed out. */
	if((currsec1 + TIMEOUT) < currsec2)
	{
		printf("Test FAILED: Timed lock did not wait long enough. (%d secs.)\n", TIMEOUT);
		printf("time before mutex locked: %d, time after mutex timed out: %d.\n", (int)currsec1, (int)currsec2);
		return PTS_FAIL;
	}

	if((currsec1 + TIMEOUT) > currsec2)
	{
		printf("Test FAILED: Timed lock did not wait long enough (%d secs.).\n", TIMEOUT);
		printf("time before mutex locked: %d, time after mutex timed out: %d.\n", (int)currsec1, (int)currsec2);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

/****************************
 *
 * Thread's start routine.
 * f1()
 *
 * *************************/
void *f1(void *parm)
{
	struct timespec timeout;

	/* Get the current time before the mutex locked. */
	currsec1 = time(NULL);

	/* Absolute time, not relative. */
	timeout.tv_sec = currsec1 + TIMEOUT;
	timeout.tv_nsec = 0;	

	printf("Timed mutex lock will block for %d seconds starting from: %d\n", TIMEOUT, (int)time(NULL));
	if(pthread_mutex_timedlock(&mutex, &timeout) != ETIMEDOUT)
	{
		perror("Error in pthread_mutex_timedlock().\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

	/* Get time after the mutex timed out in locking. */
	currsec2 = time(NULL);

  	pthread_exit(0);
  	return (void*)(0);
}
