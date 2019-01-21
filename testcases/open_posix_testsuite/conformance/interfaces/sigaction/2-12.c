/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #2 of the sigaction system call that shows
  sigaction (when used with a non-null oact pointer) changes the action
  for a signal.

  Steps:
  1. Call sigaction to set handler for SIGSEGV to use handler1
  2. Call sigaction again to set handler for SIGSEGV to use handler2,
     but this time use a non-null oarg and verify the sa_handler for
     oarg is set for handler1.
*/

#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

void handler1(int signo LTP_ATTRIBUTE_UNUSED)
{
}

void handler2(int signo LTP_ATTRIBUTE_UNUSED)
{
}

int main(void)
{
	struct sigaction act;
	struct sigaction oact;

	act.sa_handler = handler1;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	act.sa_handler = handler2;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, &oact) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (oact.sa_handler == handler1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test Failed\n");
	return PTS_FAIL;
}
