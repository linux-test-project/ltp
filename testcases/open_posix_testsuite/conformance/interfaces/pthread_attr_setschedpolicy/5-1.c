/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * Re-written by Peter W. Morreale <pmorreale AT novell DOT com>
 * 24/05/2011
 *
 * Test pthread_attr_setschedpolicy()
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Test pthread_attr_setschedpolicy for all required supported policies
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define ERR_MSG(p, f, rc) printf("Failed: %s function: %s error: %s (%u)\n", \
						p, f, strerror(rc), rc)

int set_policy(char *label, int policy)
{
	int rc;
	int status;
	pthread_attr_t attr;

	rc = pthread_attr_init(&attr);
	if (rc) {
		ERR_MSG("", "pthread_attr_init()", rc);
		return PTS_UNRESOLVED;
	}

	status = PTS_PASS;
	rc = pthread_attr_setschedpolicy(&attr, policy);
	if (rc) {
		ERR_MSG(label, "pthread_attr_setschedpolicy()", rc);
		status = PTS_FAIL;
	}

	(void)pthread_attr_destroy(&attr);

	return status;
}

int main(void)
{
	int rc;

	rc = set_policy("SCHED_FIFO", SCHED_FIFO);
	if (rc)
		goto done;

	rc = set_policy("SCHED_RR", SCHED_RR);
	if (rc)
		goto done;

	rc = set_policy("SCHED_OTHER", SCHED_OTHER);
	if (rc)
		goto done;

	printf("Test PASSED\n");

done:
	return rc;
}
