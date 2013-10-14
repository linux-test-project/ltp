/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() returns -1 on failure.
 * Simulate failure condition by sending -1 as the nsec to sleep for.
 */
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{
	struct timespec tssleepfor, tsstorage;
	int sleepnsec = -1;

	tssleepfor.tv_sec = 0;
	tssleepfor.tv_nsec = sleepnsec;
	if (nanosleep(&tssleepfor, &tsstorage) == -1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("nanosleep() did not return -1 on failure\n");
	return PTS_FAIL;
}
