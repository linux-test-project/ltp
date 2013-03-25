/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that when signo in sigqueue() is 0, error checking is
 *  still performed.

 *  Steps:

 *  Call sigqueue() with signo equal to zero but pid is 999999
 *  and verify that ESRCH error received and sigqueue() returned -1.

 *  Call sigqueue() with signo equal to zero but pid is 1 (init)
 *  and verify EPERM error received and kill() returned -1.

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

	/*
	 * ESRCH
	 */
	if (-1 == sigqueue(999999, 0, value)) {
		if (ESRCH == errno) {
			printf("ESRCH error received\n");
		} else {
			printf
			    ("sigqueue() failed on ESRCH but errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("sigqueue() did not return -1 on ESRCH\n");
		failure = 1;
	}

	if (failure) {
		printf("At least one test FAILED -- see output for status\n");
		return PTS_FAIL;
	} else {
		printf("All tests PASSED\n");
		return PTS_PASS;
	}
}
