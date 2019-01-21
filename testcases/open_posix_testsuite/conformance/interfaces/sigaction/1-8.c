/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  Rusty.Lnch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #1 of the sigaction system call that shows
  sigaction (when used with a non-null act pointer) changes the action
  for a signal.

  Steps:
  1. Initialize a global variable to indicate the signal
     handler has not been called. (A signal handler of the
     prototype "void func(int signo);" will set the global
     variable to indicate otherwise.
  2. Use sigaction to setup a signal handler for SIGILL
  3. Raise SIGILL.
  4. Verify the global indicates the signal was called.
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

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGILL, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGILL) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (handler_called) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
