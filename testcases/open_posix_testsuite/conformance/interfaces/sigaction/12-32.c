/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #12 of the sigaction system call that verifies
  signal-catching functions are executed on the current stack if the
  SA_ONSTACK flag is set in the sigaction.sa_flags parameter to the
  sigaction function call, but a new stack has not be set by sigaltstack().

  NOTE: This test case does not attempt to verify the proper operation
        of sigaltstack.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

stack_t current;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	stack_t oss;

	printf("Caught SIGFPE\n");

	if (sigaltstack(NULL, &oss) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		exit(-1);
	}

	if (oss.ss_sp != current.ss_sp || oss.ss_size != current.ss_size) {
		printf("Test FAILED\n");
		exit(-1);
	}
}

int main(void)
{
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = SA_ONSTACK;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGFPE, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sigaltstack(NULL, &current) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGFPE) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
