/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_attr_setinheritsched()
 *
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr)
 * 2.  Set schedule policy (policy) in attr to SCHED_FIFO
 * 3.  Set inheritsched to PTHREAD_EXPLICIT_SCHED in attr
 * 4.  Call pthread_create with attr
 * 5.  Call pthread_getschedparam in the created thread and get the
 *     policy value(new_policy)
 * 6.  Compare new_policy with SCHED_OTHER. SCHED_OTHER is the
 *     default policy value in the creating thread. if new_policy is
 *     equal to SCHED_OTHER, the case fails.
 *
 * - adam.li@intel.com: 2004-04-30
 *  add code pthread_attr_setschedparam(). Otherwise pthread_create()
 *  will fail with EINVAL. And, the perror() thing is not correct.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "2-2"
#define FUNCTION "pthread_attr_setinheritsched"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

const long int policy = SCHED_FIFO;
const long int old_policy = SCHED_OTHER;
void *thread_func(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc;
	int new_policy;
	pthread_t self = pthread_self();

	struct sched_param param;
	memset(&param, 0, sizeof(param));

	rc = pthread_getschedparam(self, &new_policy, &param);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_getschedparam");
		exit(PTS_UNRESOLVED);
	}
	if (new_policy == old_policy) {
		fprintf(stderr, ERROR_PREFIX "The scheduling attribute should "
			"not be inherited from creating thread \n");
		exit(PTS_FAIL);
	}
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t attr;
	int rc;
	struct sched_param sp;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_attr_setschedpolicy(&attr, policy);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy");
		exit(PTS_UNRESOLVED);
	}

	sp.sched_priority = 1;
	rc = pthread_attr_setschedparam(&attr, &sp);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam");
		exit(PTS_UNRESOLVED);
	}

	int insched = PTHREAD_EXPLICIT_SCHED;
	rc = pthread_attr_setinheritsched(&attr, insched);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setinheritsched");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_create(&new_th, &attr, thread_func, NULL);
	if (rc != 0) {
		printf("Error at pthread_create(): %s\n", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_join(new_th, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_join");
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
