/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the signal will be added to the
 signal mask before its handler is executed.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int signal_blocked = 0;

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("SIGCHLD called. Inside handler\n");
	sigset_t mask;
	sigprocmask(SIG_SETMASK, NULL, &mask);
	if (sigismember(&mask, SIGCHLD)) {
		signal_blocked = 1;
	}
}

int main(void)
{
	if (sigset(SIGCHLD, myhandler) == SIG_ERR) {
		perror("Unexpected error while using sigset()");
		return PTS_UNRESOLVED;
	}

	raise(SIGCHLD);

	if (signal_blocked != 1) {
		printf
		    ("Test FAILED: handler was called even though default was expected\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
}
