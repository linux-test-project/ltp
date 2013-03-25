/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that if the signal is the null signal (0), no signal is sent.
 *  1) Call sigqueue on the current process with the null signal.
 *  2) If process is still functional after siqueue() is called, consider
 *     the test a pass (most likely no signal was sent).
 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	union sigval value;
	value.sival_int = 0;	/* 0 is just an arbitrary value */

	if (sigqueue(getpid(), 0, value) != 0) {
		printf("Could not call sigqueue with sig = 0\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
