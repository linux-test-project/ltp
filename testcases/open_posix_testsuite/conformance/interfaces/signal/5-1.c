/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that the signal function shall return
 the function name of the last signal handler that was associated with
 sig.

 How this program tests this assertion is by setting up handlers
 SIGUSR1_handler and SIGUSR2_handler for signals SIGUSR1 and SIGUSR2
 respectively. A third call to signal() is made regarding signal SIGUSR1.
 If this call returns anything but SIGUSR1_handler, fail the test,
 otherwise the test passes.

*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

void SIGUSR1_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("do nothing useful\n");
}

void SIGUSR2_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("do nothing useful\n");
}

int main(void)
{
	if (signal(SIGUSR1, SIGUSR1_handler) == SIG_ERR) {
		perror("Unexpected error while using signal()");
		return PTS_UNRESOLVED;
	}

	if (signal(SIGUSR2, SIGUSR2_handler) == SIG_ERR) {
		perror("Unexpected error while using signal()");
		return PTS_UNRESOLVED;
	}

	if (signal(SIGUSR1, SIG_IGN) != SIGUSR1_handler) {
		printf
		    ("signal did not return the last handler that was associated with SIGUSR1\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
