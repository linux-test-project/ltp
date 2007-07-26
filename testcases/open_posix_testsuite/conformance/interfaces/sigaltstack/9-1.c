/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program tests the assertion that there are no alternate signal stacks in the new
 process image, after a successful call to one of the exec functions.

 Steps:
 - Set up a handler for signal SIGTOTEST and set the sa_flags member to SA_ONSTACK.
 - Allocate memory for the alternate signal stack
 - call sigaltstack() to define the alternate signal stack
 - Now execl() to a helper program call 9-buildonly.test.
 - The helper program will use sigaltstack to examine/obtain the current alternate signal
   stack and verify that the alternate signal stack defined here has not been carried
   over to the helper program.
*/

#define _XOPEN_SOURCE 600

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

stack_t alternate_s;

void handler (int signo) {
	printf("Inside Handler\n");
}

int main()
{

	struct sigaction act;
	act.sa_flags = SA_ONSTACK;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGTOTEST,  &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if ((alternate_s.ss_sp = (void *)malloc(SIGSTKSZ)) == NULL) {
		perror("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	alternate_s.ss_flags = 0;
	alternate_s.ss_size = SIGSTKSZ;
	
	if (sigaltstack(&alternate_s, (stack_t *)0) == -1) {
		perror("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (execl("conformance/interfaces/sigaltstack/9-buildonly.test", "9-buildonly.test", NULL) == -1) {
		perror("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

