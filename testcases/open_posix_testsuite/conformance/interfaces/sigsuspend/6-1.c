/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Assumption: The test assumes that this program is run under normal conditions,
 and not when the processor and other resources are too stressed.

 Steps:
 1. Fork() a child process. Have the child suspend itself.
 2. From the parent, send the child a SIGUSR1 signal so that the child returns from
    suspension.
 3. In the child process, make sure that sigsuspend returns a -1 and pass that info
    to the parent process.

*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Now inside signal handler\n");
}

int main(void)
{
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		/* child */

		sigset_t tempmask;

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);

		sigemptyset(&tempmask);

		if (sigaction(SIGUSR1, &act, 0) == -1) {
			perror
			    ("Unexpected error while attempting to pre-conditions");
			return 3;
		}

		printf("suspending child\n");
		if (sigsuspend(&tempmask) != -1) {
			perror("sigsuspend error");
			return 1;
		}
		printf("returned from suspend\n");

		sleep(1);
		return 2;

	} else {
		int s;
		int exit_status;

		/* parent */
		sleep(1);

		printf("parent sending child a SIGUSR1 signal\n");
		kill(pid, SIGUSR1);

		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return PTS_UNRESOLVED;
		}

		exit_status = WEXITSTATUS(s);

		printf("Exit status from child is %d\n", exit_status);

		if (exit_status == 1) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		if (exit_status == 2) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		if (exit_status == 3) {
			return PTS_UNRESOLVED;
		}

		printf
		    ("Child didn't exit with any of the expected return codes\n");
		return PTS_UNRESOLVED;
	}
}
