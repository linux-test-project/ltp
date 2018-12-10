/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that sigaltstack() returns 0 upon
 successful completion.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

stack_t alternate_s;

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

	if (sigaltstack(&alternate_s, NULL) != 0) {
		printf("Test FAILED: sigaltstack didn't return 0.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
