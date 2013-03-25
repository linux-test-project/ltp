/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Assumption: The test assumes that this program is run under normal conditions,
 and not when the processor and other resources are too stressed.

 This program checks to see that sigsuspend does not return when sent a signal
 whose action is to terminate the process.

 Steps:
 1. Fork() a child.
 2. Have the parent give the child a 1 second headstart. Have the child suspend itself.
 3. Have the parent send the child a SIGABRT signal. The default action associated with SIGABRT
    is to terminate the program. sigsuspend() should not return, and if it does, then have the
    child return a to the parent with 1 or 2.
 4. From the parent, verify that the child neither returns 1 or 2.

*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		/* child */

		sigset_t tempmask;

		sigemptyset(&tempmask);

		printf("suspending child\n");
		if (sigsuspend(&tempmask) != -1) {
			perror("sigsuspend error");
			return 1;
		}

		printf
		    ("Test FAILED: Should not have returned from sigsuspend\n");
		return 2;

	} else {
		int s;
		int exit_status;

		/* parent */
		sleep(1);

		printf("parent sending child a SIGABRT signal\n");
		kill(pid, SIGABRT);

		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return PTS_UNRESOLVED;
		}

		exit_status = WEXITSTATUS(s);

		printf("Exit status from child is %d\n", exit_status);

		if (exit_status == 1) {
			printf
			    ("Test UNRESOLVED: sigsuspend in child process was not successful\n");
			return PTS_UNRESOLVED;
		}

		if (exit_status == 2) {
			printf
			    ("Test FAILED: sigsuspend did not suspend the child\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
