/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that when pid does not exist, then sigqueue() returns -1
 *  and errno is set to [ESRCH]

 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "posixtest.h"

int main()
{
	int failure = 0;
	union sigval value;
	value.sival_int = 0; /* 0 is just an arbitrary value */

	if (-1 == sigqueue(999999, 0, value)) {
		if (ESRCH == errno) {
			printf("ESRCH error received\n");
		} else {
			printf("sigqueue() failed on ESRCH but errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("sigqueue() did not return -1 on ESRCH\n");
		failure = 1;
	}

	if (failure) {
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
