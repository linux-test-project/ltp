/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that a signal explicitly declared to execute on
 an alternate stack (using sigaltstack) shall be delivered on the alternate stack
 specified by ss.

 While it is easy to use a call like sigaltstack((stack_t *)0, &oss) to verify
 that the signal stack that the handler is being executed on is the same alternate
 signal stack ss that I defined for that signal, I do not want to rely of the
 use of oss since we're testing sigaltstack().

 Instead, what I do is declare an integer i inside the handler and verify that
 at least the address of i is located between ss.ss_sp and ss.ss_size.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

stack_t alternate_s;

void handler()
{
	int i = 0;
	if ((void *)&i < (alternate_s.ss_sp)
	    || (long)&i >=
	    ((long)alternate_s.ss_sp + (long)alternate_s.ss_size)) {

		printf
		    ("Test FAILED: address of local variable is not inside the memory allocated for the alternate signal stack\n");
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

	if (raise(SIGUSR1) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
