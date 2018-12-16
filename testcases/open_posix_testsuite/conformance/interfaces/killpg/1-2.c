/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *   Steps:
 *  1) Fork a child process.
 *  2) In the parent process, call killpg with signal SIGTOTEST for the
 *     process group id of the child. Have the parent ignore such a signal
 *     incase the process group id of the parent is the same as process
 *     group id of the child.
 *  In the child,
 *    3) Wait for signal SIGTOTEST.
 *    4) Return 1 if SIGTOTEST is found.  Return 0 otherwise.
 *  5) In the parent, return success if 1 was returned from child.
 *
 */

#define SIGTOTEST SIGUSR1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

void myhandler(int signo)
{
	(void) signo;
	_exit(1);
}

int main(void)
{
	int child_pid, child_pgid;

	if ((child_pid = fork()) == 0) {
		/* child here */
		struct sigaction act;
		act.sa_handler = myhandler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGTOTEST, &act, 0);

		/* change child's process group id */

		/*
		 * XXX: POSIX 1003.1-2001 added setpgrp(2) to BASE, but
		 * unfortunately BSD has had their own implementations for
		 * ages for compatibility reasons.
		 */
#if __FreeBSD__ || __NetBSD__ || __OpenBSD__
		setpgrp(0, 0);
#else
		setpgrp();
#endif

		sigpause(SIGABRT);

		return 0;
	} else {
		/* parent here */
		int i;
		sigignore(SIGTOTEST);

		sleep(1);
		if ((child_pgid = getpgid(child_pid)) == -1) {
			printf("Could not get pgid of child\n");
			return PTS_UNRESOLVED;
		}

		if (killpg(child_pgid, SIGTOTEST) != 0) {
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
