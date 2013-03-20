/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_getres() sets errno=EINVAL if clock_id does not
 * refer to a known clock.
 */
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "posixtest.h"

#define INVALIDCLOCKID 99999

int main(void)
{
	struct timespec res;

	if (clock_getres(INVALIDCLOCKID, &res) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno != EINVAL\n");
			return PTS_FAIL;
		}
	} else {
		printf("clock_getres() did not return -1\n");
		return PTS_UNRESOLVED;
	}

	return PTS_UNRESOLVED;
}
