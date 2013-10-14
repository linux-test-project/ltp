/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that clock_settime() sets errno to EINVAL if clock_id does not
 * specify a known clock.
 *
 * The date chosen is Nov 12, 2002 ~11:13am (date when test was first
 * written).
 */
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define TESTTIME 1037128358

#define BOGUSCLOCKID 9999

int main(void)
{
	struct timespec tpset;

	tpset.tv_sec = TESTTIME;
	tpset.tv_nsec = 0;
	if (geteuid() != 0) {
		printf("This test must be run as superuser\n");
		return PTS_UNRESOLVED;
	}
	if (clock_settime(BOGUSCLOCKID, &tpset) == -1) {
		if (EINVAL == errno) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno != EINVAL\n");
			return PTS_FAIL;
		}
	}

	printf("clock_settime() did not return -1\n");
	return PTS_UNRESOLVED;
}
