/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the function shall be executed
 when the signal occurs if the disp parameter is the address of a function.

 How this program tests this assertion is by setting up a handler
 "myhandler" for SIGCHLD, and then raising that signal. If the
 handler_called variable is anything but 1, then fail, otherwise pass.

*/

#define _XOPEN_SOURCE 600

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int handler_called = 0;

void myhandler(int signo)
{
	printf("SIGCHLD called. Inside handler\n");
	handler_called = 1;
}

int main(void)
{
	if (sigset(SIGCHLD, myhandler) == SIG_ERR) {
		perror("Unexpected error while using sigset()");
		return PTS_UNRESOLVED;
	}

	raise(SIGCHLD);

	if (handler_called != 1) {
		printf
		    ("Test FAILED: handler was called even though default was expected\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
