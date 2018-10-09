/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Assumption: The test assumes that this program is run under normal conditions,
 and not when the processor and other resources are too stressed.

 This program tries to verify two things:

 1. sigsuspend() replaces the original signal mask (containing SIGUSR1)
    with the new signal mask (containing SIGUSR2.) This can be accomplished
    by having the child call sigsuspend, and then have the parent send the
    child a SIGUSR2 signal. The signal should remain pending while as long
    as the child is suspended. How do we verify that a signal is pending?
    Well, if it wasn't for the fact that the child is suspended, we could
    have easily called the sigpending() from the child process. Because
    the child is suspended, we have to somehow verify that the signal is
    pending using only the parent process. This is acheived by having the
    parent send the child another signal, one that will cause the child to
    resume execution. If the SIGUSR2 is only delivered after sigsuspend
    returns, then that means that SIGUSR2 has in fact been pending while
    the child was suspended, and therefore that proves that sigsuspend()
    did successfully temporarily replace the original signal mask with one
    containing only SIGUSR2.

 2. The child process is suspended until the parent process delivers
    SIGUSR1. We verify this using the following rationale: Via the 3 seconds of
    sleep at the very start of the parent section of the code, the parent
    process allowed for enough time for the child process to complete execution
    and get to the "return 2" line at the very end of the child's code, but the
    parent didn't allow for any time in which the child may have been suspended.
    Because the child did receive the signal that the parent later sent before
    the child finished executing, that had to have meant that the child was
    suspended for a while during it's execution.

*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int SIGUSR1_called = 0;
int SIGUSR2_called = 0;

void handler(int signo)
{
	if (signo == SIGUSR1) {
		printf("SIGUSR1 called. Inside handler\n");
		SIGUSR1_called = 1;
		if (SIGUSR2_called == 1) {
			exit(1);
		}
	} else if (signo == SIGUSR2) {
		printf("SIGUSR2 called. Inside handler\n");
		SIGUSR2_called = 1;
		if (SIGUSR1_called == 1)
			exit(0);
		else
			exit(1);
	}
}

int main(void)
{
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		/* child */

		sigset_t tempmask, originalmask;

		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);

		sigemptyset(&tempmask);
		sigaddset(&tempmask, SIGUSR2);

		if (sigaction(SIGUSR1, &act, 0) == -1) {
			perror
			    ("Unexpected error while attempting to pre-conditions");
			return PTS_UNRESOLVED;
		}

		if (sigaction(SIGUSR2, &act, 0) == -1) {
			perror
			    ("Unexpected error while attempting to pre-conditions");
			return PTS_UNRESOLVED;
		}

		sigemptyset(&originalmask);
		sigaddset(&originalmask, SIGUSR1);
		sigprocmask(SIG_SETMASK, &originalmask, NULL);

		printf("suspending child\n");
		if (sigsuspend(&tempmask) != -1)
			perror("sigsuspend error");

		printf("returned from suspend\n");
		sleep(1);
		return 2;

	} else {
		int s;
		int exit_status;

		/* parent */
		sleep(3);

		printf("parent sending child a SIGUSR2 signal\n");
		kill(pid, SIGUSR2);

		if (SIGUSR2_called == 1) {
			printf
			    ("Test FAILED: sigsuspend did not add SIGUSR2 to the temporary mask\n");
			return PTS_FAIL;
		}
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

		if (exit_status == 1) {
			printf
			    ("Test UNRESOLVED: Either sigsuspend did not successfully block SIGUSR2, OR sigsuspend returned before handling the signal SIGUSR1\n");
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
