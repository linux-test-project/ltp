/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_getprioceiling()
 *
 * Gets the priority ceiling attribute of a mutexattr object (which was prev. created
 * by the function pthread_mutexattr_init()).
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutexattr_getprioceiling() to obtain the prioceiling.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include "posixtest.h"

static void print_pthread_error(const char *fname, int ret)
{
	printf("Unexpected error at %s(): %s\n", fname, strerror(ret));
}

int main(void)
{

	/* Make sure there is prioceiling capability. */
	/* #ifndef _POSIX_PRIORITY_SCHEDULING
	   fprintf(stderr,"prioceiling attribute is not available for testing\n");
	   return PTS_UNRESOLVED;
	   #endif */

	pthread_mutexattr_t ma;
	int prioceiling, max_prio, min_prio, ret;

	/* Initialize a mutex attributes object */
	ret = pthread_mutexattr_init(&ma);
	if (ret != 0) {
		print_pthread_error("pthread_mutexattr_init", ret);
		return PTS_UNRESOLVED;
	}

	ret = pthread_mutexattr_setprotocol(&ma, PTHREAD_PRIO_PROTECT);
	if (ret != 0) {
		print_pthread_error("pthread_mutexattr_protocol", ret);
		return PTS_UNRESOLVED;
	}

	/* Get the prioceiling mutex attr. */
	ret = pthread_mutexattr_getprioceiling(&ma, &prioceiling);
	if (ret != 0) {
		print_pthread_error("pthread_mutexattr_getprioceiling", ret);
		return PTS_UNRESOLVED;
	}

	/* Get the max and min according to SCHED_FIFO */
	max_prio = sched_get_priority_max(SCHED_FIFO);
	min_prio = sched_get_priority_min(SCHED_FIFO);

	/* Ensure that prioceiling is within legal limits. */
	if ((prioceiling < min_prio) || (prioceiling > max_prio)) {
		printf
		    ("Test FAILED: Default prioceiling %d is not compliant with SCHED_FIFO boundary.\n",
		     prioceiling);
		return PTS_FAIL;
	}

	printf("Test PASSED: Prioceiling %d\n", prioceiling);
	return PTS_PASS;
}
