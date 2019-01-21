/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the signal shall be ignored
 if the value of the func parameter is SIG_IGN.

 How this program tests this assertion is by setting up a handler
 "myhandler" for SIGCHLD. Then another call to signal() is made about
 SIGCHLD, this time with SIG_IGN as the value of the func parameter.
 SIGCHLD should be ignored now, so unless myhandler gets called when
 SIGCHLD is raised, the test passes, otherwise returns failure.

*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

static volatile int handler_called;

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("SIGCHLD called. Inside handler\n");
	handler_called = 1;
}

int main(void)
{
	if (signal(SIGCHLD, myhandler) == SIG_ERR) {
		perror("Unexpected error while using signal()");
		return PTS_UNRESOLVED;
	}

	if (signal(SIGCHLD, SIG_IGN) != myhandler) {
		perror("Unexpected error while using signal()");
		return PTS_UNRESOLVED;
	}

	raise(SIGCHLD);

	if (handler_called == 1) {
		printf
		    ("Test FAILED: handler was called even though default was expected\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
