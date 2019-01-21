/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #25 of the sigaction system call that verifies
  that when the sa_sigaction signal-catching function is entered, then
  the signal that was caught is added to the signal mask by raising that
  signal in the signal handler and verifying that the handler is not
  reentered.

  Steps:
  1. Fork a new process
  2. (parent) wait for child
  3. (child) Setup a signal handler for SIGFPE
  4. (child) raise SIGFPE
  5. (child, signal handler) increment handler count
  6. (child, signal handler) if count is 1 then raise SIGFPE
  7. (child, signal handler) if count is 2 then set error variable
  8. (child) if error is set then return -1, else return 0
  6. (parent - returning from wait) If child returned 0 then exit 0,
     otherwise exit -1.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

int handler_count = 0;
int handler_error = 0;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	static int inside_handler = 0;

	printf("SIGFPE caught\n");
	if (inside_handler) {
		printf("Signal caught while inside handler\n");
		handler_error++;
		exit(-1);
	}

	inside_handler++;
	handler_count++;

	if (handler_count == 1) {
		printf("Raising SIGFPE\n");
		raise(SIGFPE);
		printf("Returning from raising SIGFPE\n");
	}

	inside_handler--;
}

int main(void)
{
	if (fork() == 0) {
		/* child */

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		if (sigaction(SIGFPE, &act, 0) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (raise(SIGFPE) == -1) {
			perror("Unexpected error while attempting to "
			       "setup test pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (handler_error)
			return PTS_UNRESOLVED;

		return PTS_PASS;
	} else {
		int s;

		/* parent */
		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (!WEXITSTATUS(s)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
