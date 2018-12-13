/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that if the info parameter is not NULL,
 then the selected signal number shall be stored in the si_signo member.

 1. Register signal SIGTOTEST with the handler myhandler.
 2. Block signal SIGTOTEST, and then raise it causing it to become pending.
 3. Call sigwaitinfo() with only SIGTOTEST in set.
 4. Verify that info.si_signo equals SIGTOTEST.

 */

#define _XOPEN_REALTIME 1
#define SIGTOTEST SIGUSR1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

void myhandler(int signo, siginfo_t * info, void *context)
{
	printf("Inside handler\n");
}

int main(void)
{

	struct sigaction act;

	sigset_t selectset;
	siginfo_t info;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = myhandler;

	sigemptyset(&selectset);
	sigaddset(&selectset, SIGTOTEST);

	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);
	sighold(SIGTOTEST);

	raise(SIGTOTEST);

	if (sigwaitinfo(&selectset, &info) == -1) {
		perror("Call to sigwaitinfo() failed\n");
		return PTS_UNRESOLVED;
	}

	if (info.si_signo != SIGTOTEST) {
		printf("Test FAILED: The selected signal number hasn't been"
		       "stored in the si_signo member.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
