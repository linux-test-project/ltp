/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #4 of the sigaction system call that shows
  that attempting to add SIGKILL can not be added to the signal mask
  for a signal handler.

  Steps:
  1. Fork a new process
  2. (parent) wait for child
  3. (child) Setup a signal handler for SIGTSTP with SIGKILL added to
             the signal mask
  4. (child) raise SIGTSTP
  5. (child, signal handler) raise SIGKILL
  5. (child) If still alive then exit -1
  6. (parent - returning from wait) If child was killed then return success,
     otherwise fail.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	raise(SIGKILL);
	exit(0);
}

int main(void)
{
	if (fork() == 0) {
		/* child */

		/*
		 * NOTE: This block of code will return 0 for error
		 *       and anything else for success.
		 */

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGKILL);
		if (sigaction(SIGTSTP, &act, 0) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
			return PTS_PASS;
		}

		if (raise(SIGTSTP) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
		}

		return PTS_PASS;
	} else {
		int s;

		/* parent */
		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (!WIFEXITED(s)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
