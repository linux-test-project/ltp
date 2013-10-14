/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_getres() returns -1 on failure.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

#define INVALIDCLOCKID 99999

int main(void)
{
	struct timespec res;

	if (clock_getres(INVALIDCLOCKID, &res) == -1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
