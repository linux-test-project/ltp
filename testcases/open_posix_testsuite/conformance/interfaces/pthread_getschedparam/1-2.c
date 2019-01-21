/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam.li@intel.com
 *
 * The pthread_getschedparam() function shall retrieve the scheduling
 * policy and scheduling parameters for the thread whose thread ID is
 * given by thread and shall store those values in
 * policy and param, respectively. The priority value returned from
 * pthread_getschedparam() shall be
 * the value specified by the most recent pthread_setschedparam(),
 * pthread_setschedprio(), or pthread_create() call affecting the
 * target thread. It shall not reflect any temporary adjustments to
 * its priority as a result of any priority inheritance or ceiling functions.
 *
 */

 /* Set the sched parameter with pthread_setschedparam then get */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

#define ERR_MSG(f, rc)  printf("Failed: func %s rc: %s (%u)\n", \
				f, strerror(rc), rc)

void *thread_func(void* arg LTP_ATTRIBUTE_UNUSED)
{
	struct sched_param sparam;
	int policy, priority, policy_1;
	int rc;

	policy = SCHED_FIFO;
	priority = sched_get_priority_min(policy);
	sparam.sched_priority = priority;

	rc = pthread_setschedparam(pthread_self(), policy, &sparam);
	if (rc) {
		ERR_MSG("pthread_setschedparam()", rc);
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy_1, &sparam);
	if (rc != 0) {
		ERR_MSG("pthread_getschedparam()", rc);
		exit(PTS_FAIL);
	}

	if (policy_1 != policy) {
		printf("Failed:  policys:  %u != %u\n", policy_1, policy);
		exit(PTS_FAIL);
	}

	if (sparam.sched_priority != priority) {
		printf("Failed:  priorities: %u != %u\n",
		       sparam.sched_priority, priority);
		exit(PTS_FAIL);
	}

	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int rc;

	rc = pthread_create(&new_th, NULL, thread_func, NULL);
	if (rc) {
		ERR_MSG("pthread_create()", rc);
		return PTS_UNRESOLVED;
	}

	pthread_join(new_th, NULL);
	printf("Test PASSED\n");
	return PTS_PASS;
}
