/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the kill() function shall send signal sig to the process
 *  specified by pid when the process specified by pid is not the calling
 *  process.
 *  1) Fork a child process.
 *  2) In the parent process, call kill with signal SIGTOTEST for the
 *     pid of the child.
 *  In the child,
 *    3) Wait for signal SIGTOTEST.
 *    4) Return 1 if SIGTOTEST is found.  Return 0 otherwise.
 *  5) In the parent, return success if 1 was returned from child.
 *
 *  This test is only performed on one signal.  All other signals are
 *  considered to be in the same equivalence class.
 *
 *  This test makes the assumption that 1 second of sleeping on the part
 *  of the parent is enough to give the child time to start waiting for
 *  the parent's signal.  If that is not the case, this test will fail.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

#define SIGTOTEST SIGUSR1

static void myhandler(int signo)
{
	(void) signo;
	_exit(1);
}

int main(void)
{
	int pid;

	int sig;
	sigset_t set;

	if (sigemptyset(&set) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&set, SIGTOTEST) == -1) {
		perror("Error calling sigaddset\n");
		return PTS_UNRESOLVED;
	}

	pid = fork();
	if (pid == 0) {
		/* child here */
		struct sigaction act;
		act.sa_handler = myhandler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGTOTEST, &act, 0);

		if (0 != sigwait(&set, &sig)) {
			printf("Sigwait did not return 0."
				"Possible problem with sigwait function\n");
			/* FAIL */
			return 0;
		}

		if (sig != SIGTOTEST)
			/* FAIL */
			return 0;

		return 1;
	} else if (pid > 0) {
		/* parent here */
		int i;

		sleep(1);
		if (kill(pid, SIGTOTEST) != 0) {
			printf("Could not raise signal being tested\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		if (WEXITSTATUS(i)) {
			printf("Child exited normally\n");
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Child did not exit normally.\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

	} else {
		printf("Error fork() a child\n");
		return PTS_UNRESOLVED;
	}

	printf("Should have exited from parent\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
}
