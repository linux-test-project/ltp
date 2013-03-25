/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_attr_setscope()
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setscope with unsupported scope
 *     parameter
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define ERR_MSG(f, rc) printf("Failed: func: %s rc: %s (%u)\n", \
					f, strerror(rc), rc)

int main(void)
{
	int rc1;
	int rc2;
	pthread_attr_t attr;
	int status = PTS_PASS;

	rc1 = pthread_attr_init(&attr);
	if (rc1 != 0) {
		ERR_MSG("pthread_attr_init()", rc1);
		return PTS_UNRESOLVED;
	}

	rc1 = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	rc2 = pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
	if (rc1 || rc2) {
		if (rc1 && rc2) {
			/* at least one must be supported */
			ERR_MSG("pthread_attr_setscope()", rc1);
			ERR_MSG("pthread_attr_setscope()", rc2);
		}
		if (rc1 && rc1 != ENOTSUP) {
			ERR_MSG("pthread_attr_setscope()", rc1);
			status = PTS_FAIL;
		}
		if (rc2 && rc2 != ENOTSUP) {
			ERR_MSG("pthread_attr_setscope()", rc2);
			status = PTS_FAIL;
		}
	}

	pthread_attr_destroy(&attr);

	if (status == PTS_PASS)
		printf("Test PASSED\n");
	return status;
}
