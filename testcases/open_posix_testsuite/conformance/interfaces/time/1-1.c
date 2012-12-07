/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 *
 * This test case shall return PASS on returning the value of time, otherwise
 * it fails with -1.
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	time_t current_time;

	current_time = time(NULL);
	printf("%ju secs since the Epoch\n", (uintmax_t) current_time);

	if (current_time != -1) {
		puts("Test PASSED");
		return PTS_PASS;
	} else {
		puts("Test FAILED: not value for time.");
		return PTS_FAIL;
	}
}
