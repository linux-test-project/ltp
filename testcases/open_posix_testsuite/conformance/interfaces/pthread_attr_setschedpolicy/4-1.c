/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test pthread_attr_setschedpolicy()
 *
 * Fix coding style:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setschedpolicy with invalid policy
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define ERR_MSG(f, rc) printf("Failed: func: %s rc: %s (%u)\n", \
				f, strerror(rc), rc)

#define INVALIDPOLICY 999

int main(void)
{
	int rc;
	pthread_attr_t attr;
	int status = PTS_PASS;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		ERR_MSG("pthread_attr_init()", rc);
		return PTS_UNRESOLVED;
	}

	rc = pthread_attr_setschedpolicy(&attr, INVALIDPOLICY);
	if (rc != EINVAL) {
		ERR_MSG("pthread_attr_setschedpolicy()", rc);
		status = PTS_FAIL;
	}

	pthread_attr_destroy(&attr);

	if (status == PTS_PASS)
		printf("Test PASSED\n");
	return status;
}
