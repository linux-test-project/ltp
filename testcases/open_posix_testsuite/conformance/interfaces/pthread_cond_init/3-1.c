/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_init()
 * Upon successful completion, it shall return 0.
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define ERR_MSG(f, rc)  printf("Failed: func: %s rc: %s (%u)\n", \
			f, strerror(rc), rc)

int main(void)
{
	pthread_condattr_t condattr;
	pthread_cond_t cond;
	int rc;
	int status = PTS_UNRESOLVED;
	char *label;

	label = "pthread_condattr_init()";
	rc = pthread_condattr_init(&condattr);
	if (rc)
		goto done;

	label = "pthread_cond_init()";
	rc = pthread_cond_init(&cond, &condattr);
	switch (rc) {
	case 0:
		break;
	case ENOMEM:
	case EINVAL:
	case EBUSY:
	case EAGAIN:
		status = PTS_UNRESOLVED;
		goto done;
	default:
		status = PTS_FAIL;
		goto done;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

done:
	ERR_MSG(label, rc);
	return status;
}
