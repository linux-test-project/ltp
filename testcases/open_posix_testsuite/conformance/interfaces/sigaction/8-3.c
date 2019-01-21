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

int SIGUSR2_count = 0;

void SIGUSR2_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	SIGUSR2_count++;
	printf("Caught SIGUSR2\n");
}

void SIGBUS_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught SIGBUS\n");
	raise(SIGUSR2);
	if (SIGUSR2_count) {
		printf("Test FAILED\n");
		exit(-1);
	}
}

int main(void)
{
	struct sigaction act;

	act.sa_handler = SIGBUS_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGUSR2);
	if (sigaction(SIGBUS, &act, 0) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	act.sa_handler = SIGUSR2_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGUSR2, &act, 0) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGBUS) == -1) {
		perror("Unexpected error while attempting to "
		       "setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
