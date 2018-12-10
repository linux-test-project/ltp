/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if the ss_flags member is not set to SS_DISABLE,
 the stack shall be enabled, and the ss_sp and ss_size members specify the new address
 and size of the stack.

 Steps:
 - Set up a handler for signal SIGTOTEST and set the sa_flags member to SA_ONSTACK.
 - Allocate memory for the alternate signal stack
 - call sigaltstack() to define the alternate signal stack
 - raise SIGTOTEST
 - Inside the handler, use sigaltstack to examine/obtain the current alternate signal
   stack and verify that the ss_flags member of the obtained alternate signal stack is SS_DISABLE.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

static stack_t alternate_s;

void handler()
{
	stack_t handler_s;

	if (sigaltstack(NULL, &handler_s) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		exit(PTS_UNRESOLVED);
	}

	if (handler_s.ss_flags != SS_DISABLE) {
		printf
		    ("Test FAILED: The alternate stack's ss_flags member does not contain SS_DISABLE even though the alternate signal stack is disabled.\n");
		exit(PTS_FAIL);
	}
}

int main(void)
{

	struct sigaction act;
	act.sa_flags = SA_ONSTACK;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if ((alternate_s.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	alternate_s.ss_flags = SS_DISABLE;
	alternate_s.ss_size = SIGSTKSZ;

	if (sigaltstack(&alternate_s, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (raise(SIGTOTEST) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
