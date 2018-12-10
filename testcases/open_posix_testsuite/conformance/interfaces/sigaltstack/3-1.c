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
   stack and verify:
   1. The ss_sp member of the obtained alternate signal stack is equal to the ss_sp
      that we defined in the main() function.
   2. The ss_size member of the obtained alternate signal stack is equal to the ss_size
      that we defined in the main() function.
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

	if (handler_s.ss_sp != alternate_s.ss_sp) {
		printf
		    ("Test FAILED: ss_sp of the stack is not same as the defined one\n");
		exit(PTS_FAIL);
	}

	if (handler_s.ss_size != alternate_s.ss_size) {
		printf
		    ("Test FAILED: ss_size of the stack is not same as the defined one\n");
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

	alternate_s.ss_flags = 0;
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
