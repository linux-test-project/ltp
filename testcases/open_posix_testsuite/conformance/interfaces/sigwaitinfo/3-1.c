/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * This program tests the assertion that if sigwaitinfo() was called and that
   no signal in set was pending at the time of the call, then sigwaitinfo()
   shall be suspended until a signal in set becomes pending.

  Steps:
  1. In the child process, register SIGTOTEST with handler.
  2. call sigwaitinfo() with SIGTOTEST in set.
  3. From the parent process, send a SIGTOTEST using kill.
  4. Verify that the return value of the child is PTS_PASS, which indicates
     that sigwaitinfo() has returned from it's suspended state.

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
	printf("Inside dummy handler\n");
}

int main(void)
{
	pid_t pid;

	pid = fork();
	if (pid == 0) {
		/* child */
		sigset_t selectset;

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);

		if (sigaction(SIGUSR1, &act, 0) == -1) {
			perror
			    ("Unexpected error while attempting to pre-conditions");
			return PTS_UNRESOLVED;
		}

		sigemptyset(&selectset);
		sigaddset(&selectset, SIGUSR1);

		printf("Child calling sigwaitinfo()\n");

		if (sigwaitinfo(&selectset, NULL) == -1) {
			perror("Call to sigwaitinfo() failed\n");
			return PTS_UNRESOLVED;
		}

		printf("returned from sigwaitinfo\n");
		sleep(1);
		return PTS_PASS;

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

		if (!WIFEXITED(s)) {
			printf("Test FAILED: Did not exit normally\n");
			return PTS_FAIL;
		}

		exit_status = WEXITSTATUS(s);

		printf("Exit status from child is %d\n", exit_status);

		if (exit_status != PTS_PASS) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
