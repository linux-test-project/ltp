/*

 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 The resulting set shall be the union of the current set and the signal
 set pointed to by set, if the value of the argument how is SIG_BLOCK.

*/

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

static volatile int handler_called;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
}

int main(void)
{
	struct sigaction act;
	sigset_t blocked_set1, blocked_set2, pending_set;
	sigemptyset(&blocked_set1);
	sigemptyset(&blocked_set2);
	sigaddset(&blocked_set1, SIGABRT);
	sigaddset(&blocked_set2, SIGUSR2);

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGABRT, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sigaction(SIGUSR2, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_SETMASK, &blocked_set1, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to use sigprocmask.\n");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_BLOCK, &blocked_set2, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to use sigprocmask.\n");
		return PTS_UNRESOLVED;
	}

	if ((raise(SIGABRT) == -1) | (raise(SIGUSR2) == -1)) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (handler_called) {
		printf("FAIL: Signal was not blocked\n");
		return PTS_FAIL;
	}

	if (sigpending(&pending_set) == -1) {
		perror("Unexpected error while attempting to use sigpending\n");
		return PTS_UNRESOLVED;
	}

	if ((sigismember(&pending_set, SIGABRT) !=
	     1) | (sigismember(&pending_set, SIGUSR2) != 1)) {
		perror("FAIL: sigismember did not return 1\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED: signal was added to the process's signal mask\n");
	return PTS_PASS;
}
