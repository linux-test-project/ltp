/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Copyright (c) 2013, Cyril Hrubis <chrubis@suse.cz>
 *
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Steps:
 * 1. Set up a handler for signal SIGABRT, such that it is called if
 *    signal is ever raised.
 * 2. Call sighold on that SIGABRT.
 * 3. Raise a SIGABRT and verify that the signal handler was not called.
 *    Otherwise, the test exits with unresolved results.
 * 4. Call sigrelse on SIGABRT.
 * 5. Verify that the handler gets called this time.
 */

#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

static int handler_called;

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
		perror("Failed to set signal handler.");
		return PTS_UNRESOLVED;
	}

	sighold(SIGABRT);

	if (raise(SIGABRT) == -1) {
		perror("Failed to raise SIGABRT.");
		return PTS_UNRESOLVED;
	}

	if (handler_called) {
		printf("UNRESOLVED. possible problem in sigrelse\n");
		return PTS_UNRESOLVED;
	}

	if (sigrelse(SIGABRT) == -1) {
		printf("UNRESOLVED. possible problem in sigrelse\n");
		return PTS_UNRESOLVED;
	}

	nanosleep(&signal_wait_ts, NULL);

	if (handler_called) {
		printf("Test PASSED: SIGABRT removed from signal mask\n");
		return PTS_PASS;
	}
	printf("Test FAILED\n");
	return PTS_FAIL;
}
