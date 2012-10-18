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

 /* Set the sched parameter with pthread_setschedprio then get */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

void *a_thread_func()
{
	struct sched_param sparam;
	int policy, priority, policy_1;
	int rc;

	policy = SCHED_FIFO;
	priority = sched_get_priority_min(policy);
	sparam.sched_priority = priority;

	rc = pthread_setschedparam(pthread_self(), policy, &sparam);
	if (rc != 0)
	{
		printf("Error at pthread_setschedparam: rc=%d\n", rc);
		exit(PTS_UNRESOLVED);
	}
	rc = pthread_getschedparam(pthread_self(), &policy_1, &sparam);
	if (rc != 0)
	{
		printf("Error at pthread_getschedparam: rc=%d\n", rc);
		exit(PTS_UNRESOLVED);
	}
	printf("policy: %d, priority: %d\n", policy_1, sparam.sched_priority);
	if (policy_1 != policy || sparam.sched_priority != priority)
	{
		printf("pthread_getschedparam did not get the correct value\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_setschedprio(pthread_self(), priority + 1);
	if (rc != 0)
	{
		printf("Error at pthread_setschedprio: rc=%d\n", rc);
		exit(PTS_FAIL);
	}

	rc = pthread_getschedparam(pthread_self(), &policy_1, &sparam);
	if (rc != 0)
	{
		printf("Error at pthread_getschedparam: rc=%d\n", rc);
		exit(PTS_UNRESOLVED);
	}
	printf("policy: %d, priority: %d\n", policy_1, sparam.sched_priority);
	if (sparam.sched_priority != (priority + 1))
	{
		printf("Priority is not set correctly\n");
		exit(PTS_FAIL);
	}
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	pthread_join(new_th, NULL);
	printf("Test PASSED\n");
	return PTS_PASS;
}
