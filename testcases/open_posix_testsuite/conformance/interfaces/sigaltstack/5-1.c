/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that oss points to the alternate structure
 that was in effect prior to the call to sigaltstack.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

void handler()
{
	printf("Do nothing useful\n");
}

int main(void)
{

	stack_t alternate_s, current_s;
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

	if (sigaltstack(NULL, &current_s) == -1) {
		perror
		    ("Unexpected error while attempting to setup test pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (current_s.ss_sp != alternate_s.ss_sp) {
		printf
		    ("Test FAILED: ss_sp of the alternate stack is not same as the defined one\n");
		exit(PTS_FAIL);
	}

	if (current_s.ss_size != alternate_s.ss_size) {
		printf
		    ("Test FAILED: ss_size of the alternate stack is not same as the defined one\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
