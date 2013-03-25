/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_attr_getinheritsched()
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setinheritsched with inheritsched parameter
 * 3.  Call pthread_attr_getinheritsched to get the inheritsched
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_getinheritsched"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define EXPLICIT PTHREAD_EXPLICIT_SCHED
#define INHERIT PTHREAD_INHERIT_SCHED

int verify_inheritsched(pthread_attr_t * attr, int schedtype)
{
	int rc;
	int inheritsched;

	rc = pthread_attr_getinheritsched(attr, &inheritsched);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_getinheritsched");
		exit(PTS_UNRESOLVED);
	}
	switch (schedtype) {
	case INHERIT:
		if (inheritsched != INHERIT) {
			perror(ERROR_PREFIX "got wrong inheritsched param");
			exit(PTS_FAIL);
		}
		break;
	case EXPLICIT:
		if (inheritsched != EXPLICIT) {
			perror(ERROR_PREFIX "got wrong inheritsched param");
			exit(PTS_FAIL);
		}
		break;
	}
	return 0;
}

int main(void)
{
	int rc = 0;
	pthread_attr_t attr;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_setinheritsched(&attr, INHERIT);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_setinheritsched");
		exit(PTS_UNRESOLVED);
	}
	verify_inheritsched(&attr, INHERIT);

	rc = pthread_attr_setinheritsched(&attr, EXPLICIT);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_setinheritsched");
		exit(PTS_UNRESOLVED);
	}
	verify_inheritsched(&attr, EXPLICIT);

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
