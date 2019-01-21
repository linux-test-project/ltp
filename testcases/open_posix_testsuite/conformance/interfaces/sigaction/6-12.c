/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #5 of the sigaction system call that verifies
  setting the SA_INFO bit in the signal mask for SIGSEGV will result
  in sa_sigaction identifying the signal-catching function.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

static volatile int handler_called;

void handler(int signo LTP_ATTRIBUTE_UNUSED,
	siginfo_t *info LTP_ATTRIBUTE_UNUSED,
	void *context LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
}

int main(void)
{
	struct sigaction act;

	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGSTOP);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		printf("Unexpected error while attempting to setup test "
		       "pre-conditions\n");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGSEGV) == -1) {
		printf("Unexpected error while attempting to setup test "
		       "pre-conditions\n");
		return PTS_UNRESOLVED;
	}

	if (handler_called) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
