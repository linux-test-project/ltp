/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_setprotocol()
 *
 * It Shall fail if:
 * 	[ENOTSUP] The value specified by protocol is an unsupported value.
 * It may fail if:
 *      [EINVAL]  'protocol' is invalid
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_PROTOCOL -1

int main(void)
{

	pthread_mutexattr_t mta;
	int protocol = INVALID_PROTOCOL;

	int ret;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	while (protocol == PTHREAD_PRIO_NONE || protocol == PTHREAD_PRIO_INHERIT
	       || protocol == PTHREAD_PRIO_PROTECT) {
		protocol--;
	}

	/* Set the protocol to an invalid value. */
	ret = pthread_mutexattr_setprotocol(&mta, protocol);
	if ((ret == ENOTSUP) || (ret == EINVAL)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {

		printf("Test FAILED: Expected error code ENOTSUP, got %d.\n",
		       ret);
		return PTS_FAIL;
	}
}
