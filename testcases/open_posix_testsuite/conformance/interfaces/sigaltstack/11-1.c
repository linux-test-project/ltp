/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if the ss_flags member is set to something
 other than SS_DISABLE, and the ss argument is something other than a null pointer,
 then sigaltstack() shall return -1 and set errno to [EINVAL].

 Steps:
 - Set up a dummy handler for signal SIGTOTEST and set the sa_flags member to SA_ONSTACK.
 - Allocate memory for the alternate signal stack (altstack1)
 - Set the ss_flags member of the alternate stack to something other than SS_DISABLE
 - call sigaltstack() to define the alternate signal stack
 - Verify that sigaltstack() returns -1 and sets errno to [EINVAL].
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

stack_t altstack1;

void handler()
{
	printf("Just a dummy handler\n");
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

	altstack1.ss_flags = SS_DISABLE + 1;
	altstack1.ss_size = SIGSTKSZ;

	if (sigaltstack(&altstack1, NULL) != -1) {
		printf("Test FAILED: Expected return value of -1.\n");
		return PTS_FAIL;
	}

	if (errno != EINVAL) {
		printf("Test FAILED: Errno [EINVAL] was expected.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
