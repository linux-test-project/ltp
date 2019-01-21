/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the signal shall be ignored
 if the value of the dist parameter is SIG_IGN.

 How this program tests this assertion is by setting up a handler
 "myhandler" for SIGUSR1. Then another call to signal() is made about
 SIGUSR1, this time with SIG_IGN as the value of the func parameter.
 SIGUSR1 should be ignored now, so unless myhandler gets called when
 SIGUSR1 is raised, the test passes, otherwise returns failure.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

static volatile int handler_called;

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("SIGUSR1 called. Inside handler\n");
	handler_called = 1;
}

int main(void)
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = myhandler;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGUSR1, &act, 0) != 0) {
		perror("Unexpected error while using sigaction()");
		return PTS_UNRESOLVED;
	}

	if (sigset(SIGUSR1, SIG_IGN) != myhandler) {
		perror("Unexpected error while using signal()");
		return PTS_UNRESOLVED;
	}

	raise(SIGUSR1);

	if (handler_called == 1) {
		printf
		    ("Test FAILED: handler was called even though default was expected\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
