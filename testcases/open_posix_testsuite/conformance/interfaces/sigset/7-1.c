/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program tests the assertion that if disp is SIG_HOLD, then the 
 signal's disposition shall remain unchanged

 Steps:
 1. Register SIGCHLD with myhandler
 2. Add SIGCHLD to the process's signal mask using sigset with disp
    equal to SIG_HOLD
 3. raise SIGCHLD
 4. remove SIGCHLD from the signal mask
 5. Verify that the original disposition hasn't been changed, by making
    sure that SIGCHLD is still handled by myhandler.
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

int main()
{
	sigset_t pendingset;
	struct sigaction act;
	act.sa_handler = myhandler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGCHLD, &act, 0) != 0) {
                perror("Unexpected error while using sigaction()");
               	return PTS_UNRESOLVED;
        }

        if (sigset(SIGCHLD,SIG_HOLD) != SIG_HOLD) {
                perror("Unexpected error while using sigset()");
               	return PTS_UNRESOLVED;
        }

	raise(SIGCHLD);
	
        if (sigpending(&pendingset) == -1) {
                printf("Error calling sigpending()\n");
                return PTS_UNRESOLVED;
        }

        if (sigismember(&pendingset, SIGCHLD) != 1) {
		printf("Test UNRESOLVED: Signal SIGCHLD was not successfully blocked\n");
		return PTS_UNRESOLVED;
	}

	sigrelse(SIGCHLD);

	if (handler_called != 1) {
		printf("Test FAILED: Signal wasn't delivered even though it was removed from the signal mask\n");
		return PTS_FAIL;
	}
	return PTS_PASS;
} 
