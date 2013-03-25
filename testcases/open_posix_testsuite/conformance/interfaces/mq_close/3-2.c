/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
  mq_close test plan:
  Attempt to close a invalid descriptor and verify that returns
  the correct error.
 */

#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	if (mq_close((mqd_t) - 1) != -1) {
		printf("mq_close() did not return -1 on invalid descriptor\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (errno != EBADF) {
		printf("errno != EBADF on invalid descriptor\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
