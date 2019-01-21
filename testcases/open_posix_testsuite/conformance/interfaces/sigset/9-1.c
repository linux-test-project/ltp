/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that sigset shall return the signal's
 previous disposition if signal had not been blocked.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("SIGUSR1 called. Inside handler\n");
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

	if (sigset(SIGUSR1, SIG_DFL) != myhandler) {
		printf
		    ("Test FAILED: sigset didn't return myhandler even though it was SIGUSR1's original disposition\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
