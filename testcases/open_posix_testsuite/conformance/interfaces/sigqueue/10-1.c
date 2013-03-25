/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that when signo in sigqueue() is invalid, then -1 is returned
 *  and errno is set to [EINVAL]

 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "posixtest.h"

int main(void)
{
	int failure = 0;
	union sigval value;
	value.sival_int = 0;	/* 0 is just an arbitrary value */

	if (-1 == sigqueue(getpid(), -1, value)) {
		if (EINVAL == errno) {
			printf("EINVAL error received\n");
		} else {
			printf
			    ("sigqueue() failed on EINVAL but errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("sigqueue() did not return -1 on EINVAL\n");
		failure = 1;
	}

	if (failure) {
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
