/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if ss_flags is set to SS_DISABLE, then
 the alternate stack is disabled.

 Steps:
 - Set up a handler for signal SIGTOTEST and set the sa_flags member to SA_ONSTACK.
 - Using the first call to sigaltstack(), get the process's current stack info (original_s.)
 - Allocate memory for the alternate signal stack (alternate_s.)
 - call sigaltstack() to define the alternate signal stack, and set ss_flags to SS_DISABLE
 - raise SIGTOTEST
 - Inside the handler, use sigaltstack() to obtain the current stack that the handler is
   executing on (handler_s) and verify that:
   1. The ss_sp member of the handler_s is equal to that of original_s.
   2. The ss_size member of the hanlder_s is equal to that of original_s.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

static stack_t alternate_s, original_s;

void handler()
{

	stack_t handler_s;

	if (sigaltstack(NULL, &handler_s) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		exit(PTS_UNRESOLVED);
	}

	if (handler_s.ss_sp != original_s.ss_sp) {
		printf
		    ("Test FAILED: ss_sp of the handler's stack changed even though SS_DISABLE was set\n");
		exit(PTS_FAIL);
	}

	if (handler_s.ss_size != original_s.ss_size) {
		printf
		    ("Test FAILED: ss_size of the handler's stack changed even though SS_DISABLE was set\n");
		exit(PTS_FAIL);
	}

}

int main(void)
{

	struct sigaction act;
	act.sa_flags = SA_ONSTACK;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGUSR1, &act, 0) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sigaltstack(NULL, &original_s) == -1) {
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

	if (raise(SIGUSR1) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
