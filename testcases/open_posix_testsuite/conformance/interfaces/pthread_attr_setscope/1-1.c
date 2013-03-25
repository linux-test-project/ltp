/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_attr_setscope()
 *
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr)
 * 2.  sets the contentionscope to attr
 * 3.  create a thread with the attr
 * 4.  Get the contentionscope value in the created thread
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_setscope"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define CONSCOPE PTHREAD_SCOPE_SYSTEM

void *thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t attr;
	int cscope;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_setscope(&attr, CONSCOPE);
	if (rc != 0) {
		perror(ERROR_PREFIX "PTHREAD_SCOPE_SYSTEM is not supported");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_create(&new_th, &attr, thread_func, NULL);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_create");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_getscope(&attr, &cscope);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_getscope");
		exit(PTS_UNRESOLVED);
	}

	if (cscope != CONSCOPE) {
		fprintf(stderr, ERROR_PREFIX "The contentionscope is not "
			"correct \n");
		exit(PTS_FAIL);
	}

	rc = pthread_join(new_th, NULL);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_join");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
