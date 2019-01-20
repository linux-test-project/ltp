/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_exit()
 *
 * Any destructors for thread_specific data will be called when
 * pthread_exit is called
 *
 * Steps:
 * 1. Create a new thread.
 * 2. Create thread specific data, with a destructor in the thread
 * 3. Call pthread_exit in the thread.
 * 4. Make sure that the destructor was called
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "pthread_exit"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

/* Flag to indicate that the destructor was called */
int cleanup_flag = 0;

void destructor(void *tmp LTP_ATTRIBUTE_UNUSED)
{
	cleanup_flag = 1;
}

/* Thread's function. */
void *a_thread_func(void *tmp LTP_ATTRIBUTE_UNUSED)
{
	pthread_key_t key;
	int value = 1;
	int rc = 0;

	rc = pthread_key_create(&key, destructor);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_key_create\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_setspecific(key, &value);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setspecific\n");
		exit(PTS_UNRESOLVED);
	}

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int rc = 0;

	/* Create a new thread. */
	rc = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to return */
	rc = pthread_join(new_th, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		return PTS_UNRESOLVED;
	}

	if (cleanup_flag != 1) {
		printf("Test FAIL: Destructor was not called.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
