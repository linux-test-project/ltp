/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that when  pthread_cond_destroy()
 *   is called on a cond that some thread is waiting, then it returns
 *   EBUSY

 * Steps:
 * 1. Create a condvar 
 * 2. Create a thread and make it wait on the condvar
 * 3. Try to destroy the cond var in main
 * 4. Check that pthread_cond_destroy returns EBUSY
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "pthread_cond_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

/* cond used by the two threads */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;		

/* cond used by the two threads */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;		

void * thread(void *tmp)
{	
	int    rc = 0;

	/* acquire the mutex */
	rc  = pthread_mutex_lock(&mutex);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* Wait on the cond var. This will not return, as nobody signals*/
	rc = pthread_cond_wait(&cond, &mutex);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_cond_wait\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_mutex_unlock(&mutex);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_mutex_unlock\n");
		exit(PTS_UNRESOLVED);
	}
	return NULL;
}

int main()
{
	pthread_t low_id;
	int       rc = 0;
	
	/* Create a new thread with default attributes */
	rc = pthread_create(&low_id, NULL, thread, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		exit(PTS_UNRESOLVED);
	}

	/* Let the other thread run */
	sleep(2);

	/* Try to destroy the cond var. This should return an error */
	rc = pthread_cond_destroy(&cond);
	if(rc != EBUSY) {
		printf(ERROR_PREFIX "Test PASS: Expected %d(EBUSY) got %d, "
			"though the standard states 'may' fail\n", EBUSY, rc);

		exit(PTS_PASS);
	}

	printf("Test PASS\n");
	exit(PTS_PASS);
}
