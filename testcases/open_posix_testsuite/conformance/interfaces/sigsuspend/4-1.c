/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Assumption: The test assumes that this program is run under normal conditions,
 and not when the processor and other resources are too stressed.

 Steps:
 1. Fork() a child process. In the child, add SIGUSR1 to the process's signal mask.
    This is its original signal mask. Now suspend the child, passing sigsuspend another
    signal mask. One that doesn't contain SIGUSR1, but contains SIGUSR2.
 2. From the parent, send the child a SIGUSR1 signal so that the child returns from
    suspension.
 3. Once the sigsuspend returns, have the child probe the signal mask using the is_changed()
    function which basically verifies that the signal mask is restored to what it was originally
    before the call to sigsuspend. I.e. SIGUSR1 in the mask and SIGUSR2 not in the mask. Have
    the child return to the parent process with:
    - a return value of 1 if the original signal mask was not restored, or
    - a return value of 0 if the original signal mask was successfully restored.
 4. Finally from the parent, return a PTS_PASS if received the return value of the child was not
    a 1.

*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
}

int is_changed(sigset_t set, int sig)
{

	int i;
	int siglist[] = { SIGABRT, SIGALRM, SIGBUS, SIGCHLD,
		SIGCONT, SIGFPE, SIGHUP, SIGILL, SIGINT,
		SIGPIPE, SIGQUIT, SIGSEGV,
		SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU,
		SIGUSR1, SIGUSR2,
#ifdef SIGPOLL
		SIGPOLL,
#endif
#ifdef SIGPROF
		SIGPROF,
#endif
		SIGSYS,
		SIGTRAP, SIGURG, SIGVTALRM, SIGXCPU, SIGXFSZ
	};

	if (sigismember(&set, sig) != 1) {
		return 1;
	}
	for (i = 0; i < (int)NUMSIGNALS; i++) {
		if ((siglist[i] != sig)) {
			if (sigismember(&set, siglist[i]) != 0) {
				return 1;
			}
		}
	}
	return 0;
}

int main(void)
{
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		/* child */

		sigset_t tempmask, originalmask, currentmask;

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

		sigemptyset(&originalmask);
		sigaddset(&originalmask, SIGUSR1);
		sigprocmask(SIG_SETMASK, &originalmask, NULL);

		printf("suspending child\n");
		if (sigsuspend(&tempmask) != -1)
			perror("sigsuspend error");

		printf("returned from suspend\n");

		sigprocmask(SIG_SETMASK, NULL, &currentmask);

		if (is_changed(currentmask, SIGUSR1) != 0) {
			printf
			    ("signal mask was not restored properly after sigsuspend returned\n");
			return 1;
		}
		return 0;

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

		if (exit_status == 1) {
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
