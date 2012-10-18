/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * pthread_attr_setschedparam()

 * 1. Create a pthread_attr object and initialize it
 * 2. Set the policy and an invalid priority in that object

 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "pthread_attr_setschedparam"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define FIFOPOLICY SCHED_FIFO
#define PRIORITY_OFFSET 1000

int main()
{
	pthread_attr_t         attr;
	int                    rc=0;
	int                    policy = FIFOPOLICY;
	struct sched_param     param;
	int                    priority;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_setschedpolicy(&attr, policy);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_FAIL);
	}

	priority = sched_get_priority_max(policy);
	if (priority == -1) {
		printf(ERROR_PREFIX "sched_priority_get_max\n");
		exit(PTS_FAIL);
	}

	param.sched_priority = priority + PRIORITY_OFFSET;
	rc = pthread_attr_setschedparam(&attr, &param);
	if ((rc != EINVAL) && (rc != ENOTSUP)) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam did not fail\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	exit(PTS_PASS);
}
