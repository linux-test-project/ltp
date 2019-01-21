/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #4 of the sigaction system call that shows
  that attempting to add SIGSTOP can not be added to the signal mask
  for a signal handler.

  Steps:
  1. Fork a new process
  2. (parent) wait for child
  3. (child) Setup a signal handler for SIGXCPU with SIGSTOP added to
             the signal mask
  4. (child) raise SIGXCPU
  5. (child, signal handler) raise SIGSTOP
  5. (child) If still around then return -1
  6. (parent - returning from wait) If child was stopped then return
     kill the child and return success, otherwise fail.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("About to stop child\n");
	raise(SIGSTOP);
	printf("Child has continued\n");
	exit(0);
}

int main(void)
{
	pid_t pid;
	if ((pid = fork()) == 0) {
		/* child */

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGSTOP);
		if (sigaction(SIGXCPU, &act, 0) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (raise(SIGXCPU) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
		}

		return PTS_UNRESOLVED;
	} else {
		int s;

		/* parent */
		if (waitpid(pid, &s, WUNTRACED) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (WIFSTOPPED(s)) {
			printf("Test PASSED\n");
			kill(pid, SIGKILL);
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
