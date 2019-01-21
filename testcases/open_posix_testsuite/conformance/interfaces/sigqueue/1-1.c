/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that the sigqueue() function shall send signal signo and value
    to the child process specified by pid.
 *  1) Fork a child process.
 *  2) In the parent process, call sigqueue with signal SIGTOTEST and
 *     value VALTOTEST for the pid of the child
 *  In the child,
 *  3) Wait for signal SIGTOTEST.
 *  4) Return 1 if SIGTOTEST and SIGTOVAL are the values of signo and
       info->si_value.sival_intel respectively. Have the child return 0 otherwise.
 *  5) In the parent, return success if 1 was returned from child.
 *
 */

#define _XOPEN_REALTIME 1
#define SIGTOTEST SIGRTMIN
#define VALTOTEST 100		/* Application-defined value sent by sigqueue */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

void myhandler(int signo, siginfo_t *info, void *context LTP_ATTRIBUTE_UNUSED)
{
	if (signo == SIGTOTEST && info->si_value.sival_int == VALTOTEST) {
		printf
		    ("sigqueue()'s signo and value parameters were passed to the child process.\n");
		exit(1);
	}
}

int main(void)
{
	int pid;

	if ((pid = fork()) == 0) {
		/* child here */
		struct sigaction act;
		act.sa_flags = SA_SIGINFO;
		act.sa_sigaction = myhandler;
		sigemptyset(&act.sa_mask);
		sigaction(SIGTOTEST, &act, 0);

		while (1) {
			sleep(1);
		}
		printf("shouldn't be here\n");
		return 0;
	} else {
		/* parent here */
		int i;
		union sigval value;
		value.sival_int = VALTOTEST;

		sleep(1);
		if (sigqueue(pid, SIGTOTEST, value) != 0) {
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
	}

	printf("Should have exited from parent\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
}
