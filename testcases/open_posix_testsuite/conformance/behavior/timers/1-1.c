/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that TIMER_MAX timers can be created.
 *
 * For this test, clock CLOCK_REALTIME will be used.
 */

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	timer_t tid;
	int i;
	long scTIMER_MAX = 0;

	scTIMER_MAX = sysconf(_SC_TIMER_MAX);

	for (i = 0; i < scTIMER_MAX; i++) {
		if (timer_create(CLOCK_REALTIME, NULL, &tid) == -1) {
			printf("[%d] timer_create() did not return success: "
			       "%s\n", i, strerror(errno));
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
