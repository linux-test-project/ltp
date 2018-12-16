/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

* This sample test aims to check the following assertion:
*
* The child process is created with no pending signal

* The steps are:
* -> block SIGUSR1 and SIGUSR2
* -> send those signals and wait they are pending
* -> fork
* -> check the signals are blocked but not pending in the new process.

* The test fails if the signals are pending or if
* they are not blocked (this counters assertion 2).

*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <signal.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

int main(void)
{
	int ret, status;
	pid_t child, ctl;

	sigset_t mask, pending;

	output_init();

	/* block SIGUSR1 and SIGUSR2 */
	ret = sigemptyset(&mask);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to initialize signal set");
	}

	ret = sigaddset(&mask, SIGUSR1);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to add SIGUSR1 to signal set");
	}

	ret = sigaddset(&mask, SIGUSR2);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to add SIGUSR2 to signal set");
	}

	ret = sigprocmask(SIG_BLOCK, &mask, NULL);

	if (ret != 0) {
		UNRESOLVED(errno, "Sigprocmask failed");
	}

	/* Make the signals pending */
	ret = kill(getpid(), SIGUSR1);

	if (ret != 0) {
		UNRESOLVED(errno, "failed to kill with SIGUSR1");
	}

	ret = kill(getpid(), SIGUSR2);

	if (ret != 0) {
		UNRESOLVED(errno, "failed to kill with SIGUSR2");
	}

	do {
		ret = sigpending(&pending);

		if (ret != 0) {
			UNRESOLVED(errno,
				   "failed to examine pending signal set");
		}

		ret = sigismember(&pending, SIGUSR1);

		if (ret < 0) {
			UNRESOLVED(errno,
				   "Unable to check signal USR1 presence");
		}

		if (ret == 1) {
			ret = sigismember(&pending, SIGUSR2);

			if (ret < 0) {
				UNRESOLVED(errno,
					   "Unable to check signal USR2 presence");
			}
		}
	}
	while (ret != 1);

#if VERBOSE > 0

	output("SIGUSR1 and SIGUSR2 are pending, we can fork\n");

#endif

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		/* Examine the current blocked signal set. USR1 & USR2 shall be present */
		ret = sigprocmask(0, NULL, &mask);

		if (ret != 0) {
			UNRESOLVED(errno, "Sigprocmask failed in child");
		}

		ret = sigismember(&mask, SIGUSR1);

		if (ret < 0) {
			UNRESOLVED(errno,
				   "Unable to check signal USR1 presence");
		}

		if (ret == 0) {
			FAILED
			    ("The new process does not mask SIGUSR1 as its parent");
		}

		ret = sigismember(&mask, SIGUSR2);

		if (ret < 0) {
			UNRESOLVED(errno,
				   "Unable to check signal USR2 presence");
		}

		if (ret == 0) {
			FAILED
			    ("The new process does not mask SIGUSR2 as its parent");
		}
#if VERBOSE > 0
		output("SIGUSR1 and SIGUSR2 are blocked in child\n");

#endif

		/* Examine pending signals */
		ret = sigpending(&pending);

		if (ret != 0) {
			UNRESOLVED(errno,
				   "failed to examine pending signal set in child");
		}

		ret = sigismember(&pending, SIGUSR1);

		if (ret < 0) {
			UNRESOLVED(errno,
				   "Unable to check signal USR1 presence");
		}

		if (ret != 0) {
			FAILED
			    ("The new process was created with SIGUSR1 pending");
		}

		ret = sigismember(&pending, SIGUSR2);

		if (ret < 0) {
			UNRESOLVED(errno,
				   "Unable to check signal USR2 presence");
		}

		if (ret != 0) {
			FAILED
			    ("The new process was created with SIGUSR2 pending");
		}
#if VERBOSE > 0
		output("SIGUSR1 and SIGUSR2 are not pending in child\n");

#endif

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
