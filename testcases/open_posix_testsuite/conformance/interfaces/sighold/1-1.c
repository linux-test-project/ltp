/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Steps:
 1. Set up a handler for signal SIGABRT, such that it is called if signal is ever raised.
 2. Call sighold on that SIGABRT.
 3. Raise a SIGABRT and verify that the signal handler was not called.

*/


#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

static int handler_called = 0;

static void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
}

int main(void)
{
	struct sigaction act;
	struct timespec signal_wait_ts = {0, 100000000};

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGABRT, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sighold(SIGABRT) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGABRT) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	nanosleep(&signal_wait_ts, NULL);

	if (handler_called) {
		printf("FAIL: Signal was not blocked\n");
		return PTS_FAIL;
	}

	printf("Test PASSED: signal was blocked\n");
	return PTS_PASS;
}
