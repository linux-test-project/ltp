/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  Rusty.Lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test assertion #11 by verifying that SIGCHLD signals are sent to a parent
  when their children are continued after being stopped.

  NOTE: This is only required to work if the XSI options are implemented.
 * 12/18/02 - Adding in include of sys/time.h per
 *            rodrigc REMOVE-THIS AT attbi DOT com input that it needs
 *            to be included whenever the timeval struct is used.
 *
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define NUMSTOPS 2

static volatile int child_continued;
static volatile int waiting = 1;

static void handler(int signo PTS_ATTRIBUTE_UNUSED, siginfo_t *info,
	void *context PTS_ATTRIBUTE_UNUSED)
{
	if (info && info->si_code == CLD_CONTINUED) {
		printf("Child has been stopped\n");
		waiting = 0;
		child_continued++;
	}
}

int main(void)
{
	pid_t pid;
	struct sigaction act;
	struct timeval tv;

	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, 0);

	if ((pid = fork()) < 0) {
		printf("fork() did not return success\n");
		return PTS_UNRESOLVED;
	} else if (pid == 0) {
		/* child */
		while (1) {
			/* wait forever, or until we are
			   interrupted by a signal */
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);
		}
		return 0;
	} else {
		/* parent */
		int s;
		int i;

		/* delay to allow child to get into select call */
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tv);

		for (i = 0; i < NUMSTOPS; i++) {
			struct timeval tv;
			printf("--> Sending SIGSTOP\n");
			kill(pid, SIGSTOP);

			/*
			   Don't let the kernel optimize away queued
			   SIGSTOP/SIGCONT signals.
			 */
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			select(0, NULL, NULL, NULL, &tv);

			printf("--> Sending SIGCONT\n");
			waiting = 1;
			kill(pid, SIGCONT);
			while (waiting) {
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				if (!select(0, NULL, NULL, NULL, &tv))
					break;
			}

		}

		kill(pid, SIGKILL);
		waitpid(pid, &s, 0);
	}

	if (child_continued == NUMSTOPS) {
		printf("Test PASSED\n");
		printf
		    ("In the section of the POSIX spec that describes the SA_NOCLDSTOP flag in the sigaction() interface "
		     "it is specified that if the SA_NOCLDSTOP flag is not set in sa_flags, then a SIGCHLD and a SIGCHLD "
		     "signal **MAY** be generated for the calling process whenever any of its stopped child processes are continued. "
		     "Because of that, this test will PASS either way, but note that the signals implementation you are currently "
		     "run this test on DOES choose to send a SIGCHLD signal whenever any of its stopped child processes are "
		     "continued. Again, this is not a bug because of the existence of the word *MAY* in the spec.\n");
		return PTS_PASS;
	}

	printf("Test PASSED\n");

	printf
	    ("In the section of the POSIX spec that describes the SA_NOCLDSTOP flag in the sigaction() interface "
	     "it is specified that if the SA_NOCLDSTOP flag is not set in sa_flags, then a SIGCHLD and a SIGCHLD "
	     "signal **MAY** be generated for the calling process whenever any of its stopped child processes are continued. "
	     "Because of that, this test will PASS either way, but note that the signals implementation you are currently "
	     "run this test on chooses NOT TO send a SIGCHLD signal whenever any of its stopped child processes are "
	     "continued. Again, this is not a bug because of the existence of the word *MAY* in the spec.\n");
	return PTS_PASS;
}
