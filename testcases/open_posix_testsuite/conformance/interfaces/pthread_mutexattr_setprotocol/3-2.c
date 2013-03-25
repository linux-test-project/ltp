/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_setprotocol()
 *
 * It may fail if:
 * 	[EINVAL] The value specified by 'attr' is invalid.
 *
 * Steps:
 * 1. Call pthread_mutexattr_setprotocol with an uninitialized pthread_mutexattr_t object.
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{

	pthread_mutexattr_t mta;
	int ret;

	/* Set the protocol to an invalid value. */
	ret = pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_NONE);
	if (ret == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (ret == 0) {
		printf
		    ("Test PASSED: NOTE*: Expected error code EINVAL, got %d, though standard states 'may' fail.\n",
		     ret);
		return PTS_PASS;
	} else {
		printf
		    ("Test FAILED: Incorrect return code %d.  Expected EINVAL or 0.\n",
		     ret);
		return PTS_FAIL;
	}
}
