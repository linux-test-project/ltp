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
 - Allocate memory for the alternate signal stack (altstack1)
 - call sigaltstack() to define the alternate signal stack
 - raise SIGTOTEST
 - Inside the handler, try to change the alternate signal stack to altstack2, and verify
   that the attempt fails.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

static stack_t altstack1;

void handler()
{
	stack_t altstack2;

	if ((altstack2.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		exit(PTS_UNRESOLVED);
	}

	altstack2.ss_flags = 0;
	altstack2.ss_size = SIGSTKSZ;

	if (sigaltstack(&altstack2, NULL) != -1) {
		printf
		    ("Test FAILED: Attempt to set change alternate stack while inside handler succeeded.\n");
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

	if ((altstack1.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	altstack1.ss_flags = 0;
	altstack1.ss_size = SIGSTKSZ;

	if (sigaltstack(&altstack1, NULL) == -1) {
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
