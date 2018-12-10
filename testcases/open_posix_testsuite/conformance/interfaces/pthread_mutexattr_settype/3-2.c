/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_mutexattr_settype()
 *
 * PTHREAD_MUTEX_ERRORCHECK

 * Provides errorchecking.  A thread attempting to relock this mutex without unlocking it
 * first will return with an error.  A thread attempting to unlock a mutex which another
 * thread has locked will return with an error.  A thread attempting to unlock an unlocked
 * mutex will return with an error.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2   Set the 'type' of the mutexattr object to PTHREAD_MUTEX_ERRORCHECK.
 * 3.  Create a mutex with that mutexattr object.
 * 4.  Lock the mutex.
 * 5.  Create a thread, and in that thread, attempt to unlock the mutex. It should return an
 *     error.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_t thread1;
pthread_mutex_t mutex;
pthread_mutexattr_t mta;

int ret;			/* Return value of the thread unlocking the mutex. */

void *a_thread_func()
{
	/* Try to unlock the mutex that main already locked. */
	ret = pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

int main(void)
{

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set the 'type' attribute to be PTHREAD_MUTEX_ERRORCHECK  */
	if (pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK) != 0) {
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}

	/* Initialize the mutex with that attribute obj. */
	if (pthread_mutex_init(&mutex, &mta) != 0) {
		perror("Error initializing the mutex.\n");
		return PTS_UNRESOLVED;
	}

	/* Lock the mutex. */
	if (pthread_mutex_lock(&mutex) != 0) {
		perror("Error locking the mutex first time around.\n");
		return PTS_UNRESOLVED;
	}

	/* Create the thread that will try to unlock the mutex we just locked. */
	if (pthread_create(&thread1, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating a thread.\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for that thread to end execution */
	pthread_join(thread1, NULL);

	if (ret == 0) {
		printf
		    ("Test FAILED: Expected an error when trying to unlock a mutex that was locked by another thread.  Returned 0 instead.\n");
		return PTS_FAIL;
	}

	/* cleanup */
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);

	if (pthread_mutexattr_destroy(&mta)) {
		perror("Error at pthread_mutexattr_destroy().\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
