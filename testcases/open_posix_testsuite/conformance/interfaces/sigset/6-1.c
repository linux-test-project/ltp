/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if disp is SIG_HOLD, then the
 signal shall be added to the process's signal mask.

 Steps:
 1. Register SIGCHLD with myhandler
 2. Add SIGCHLD to the process's signal mask using sigset with disp
    equal to SIG_HOLD
 3. raise SIGCHLD
 4. Verify that SIGCHLD is pending.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define ERR_MSG(f, rc) printf("Failed: func: %s rc: %u errno: %s\n", \
						f, rc, strerror(errno))

void myhandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("SIGCHLD called. Inside handler\n");
}

int main(void)
{
	sigset_t pendingset;
	struct sigaction act;
	act.sa_handler = myhandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	int rc;

	rc = sigaction(SIGCHLD, &act, 0);
	if (rc) {
		ERR_MSG("sigaction()", rc);
		return PTS_UNRESOLVED;
	}

	if (sigset(SIGCHLD, SIG_HOLD) == SIG_ERR) {
		perror("Unexpected error while using sigset()");
		return PTS_UNRESOLVED;
	}

	raise(SIGCHLD);

	rc = sigpending(&pendingset);
	if (rc) {
		ERR_MSG("sigpending()", rc);
		return PTS_UNRESOLVED;
	}

	if (sigismember(&pendingset, SIGCHLD) != 1) {
		printf("Test FAILED: Signal SIGCHLD wasn't hold.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
