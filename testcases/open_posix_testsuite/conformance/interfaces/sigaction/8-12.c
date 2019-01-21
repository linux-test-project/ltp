/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #8 of the sigaction system call that verifies
  that if signals in the sa_mask (passed in the sigaction struct of the
  sigaction function call) are added to the process signal mask during
  execution of the signal-catching function.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

int SIGQUIT_count = 0;

void SIGQUIT_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	SIGQUIT_count++;
	printf("Caught SIGQUIT\n");
}

void SIGSEGV_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught SIGSEGV\n");
	raise(SIGQUIT);
	if (SIGQUIT_count) {
		printf("Test FAILED\n");
		exit(-1);
	}
}

int main(void)
{
	struct sigaction act;

	act.sa_handler = SIGSEGV_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGQUIT);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	act.sa_handler = SIGQUIT_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGQUIT, &act, 0) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGSEGV) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
