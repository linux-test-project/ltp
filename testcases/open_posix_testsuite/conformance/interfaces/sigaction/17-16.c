/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test assertion #17 by verifying that select returns -1 with
  errno set to EINTR if a handler for the SIGTTOU signal is setup with
  the SA_RESTART flag cleared.
 * 12/18/02 - Adding in include of sys/time.h per
 *            rodrigc REMOVE-THIS AT attbi DOT com input that it needs
 *            to be included whenever the timeval struct is used.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

volatile sig_atomic_t wakeup = 1;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught SIGTTOU\n");
	wakeup++;
}

int main(void)
{
	pid_t pid;
	struct timeval tv;

	if ((pid = fork()) == 0) {
		/* child */
		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGTTOU, &act, 0);

		while (wakeup == 1) {
			tv.tv_sec = 3;
			tv.tv_usec = 0;
			if (select(0, NULL, NULL, NULL, &tv) == -1 &&
			    errno == EINTR) {
				perror("select");
				return PTS_PASS;
			}
		}

		return PTS_FAIL;
	} else {
		/* parent */
		int s;

		/*
		   There is a race condition between the parent
		   process sending the SIGTTOU signal, and the
		   child process being inside the 'select' function
		   call.

		   I could not find a pure POSIX method for determining
		   the state of the child process, so I just added a delay
		   so that the test is valid in most conditions.  (The
		   problem is that it would be perfectly legal for a
		   POSIX conformant OS to not schedule the child process
		   for a long time.)
		 */
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tv);

		kill(pid, SIGTTOU);
		waitpid(pid, &s, 0);
		if (WEXITSTATUS(s) == PTS_PASS) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
