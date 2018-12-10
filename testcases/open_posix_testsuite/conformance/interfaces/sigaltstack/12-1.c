/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if the ss_size is equal to
 something smaller that MINSIGSTKSZ, then sigaltstack() shall return -1
 and set errno to [ENOMEM].

 Steps:
 - Set up a dummy handler for signal SIGTOTEST and set the sa_flags member to SA_ONSTACK.
 - Allocate memory for the alternate signal stack (altstack1), but make it one byte shorter
   than MINSIGSTKSZ
 - call sigaltstack() to define the alternate signal stack
 - Verify that sigaltstack() returns -1 and sets errno to [ENOMEM].
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

	altstack1.ss_flags = 0;
	/* use value low enough for all kernel versions
	 * avoid using MINSIGSTKSZ defined by glibc as it could be different
	 * from the one in kernel ABI
	 */
	altstack1.ss_size = 2048 - 1;

	if (sigaltstack(&altstack1, NULL) != -1) {
		printf("Test FAILED: Expected return value of -1.\n");
		return PTS_FAIL;
	}

	if (errno != ENOMEM) {
		printf("Test FAILED: Errno [ENOMEM] was expected.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
