/*
* Copyright (c) 2003, Intel Corporation. All rights reserved.
* Created by:  salwan.searty REMOVE-THIS AT intel DOT com
* This file is licensed under the GPL license.  For the full content
* of this license, see the COPYING file at the top level of this
* source tree.

Go through all the signals (with the exception of SIGKILL and SIGSTOP
since they cannot be added to a process's signal mask) and add each one
to the signal mask. Every time a signal gets added to the signal mask
(using the pthread_sigmask() function),  make sure that all signals added
before it in preceding iterations before it, exist in the old signal set
returned by the pthread_sigmask functions.

*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

#define NUMSIGNALS (sizeof(siglist) / sizeof(siglist[0]))

static void *a_thread_func()
{
	sigset_t oactl, tempset;
	unsigned int i, j;
	int test_failed = 0;

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

	for (i = 0; i < (int)NUMSIGNALS; i++) {
		sigemptyset(&oactl);
		sigemptyset(&tempset);
		sigaddset(&tempset, siglist[i]);
		pthread_sigmask(SIG_BLOCK, &tempset, &oactl);

		if (i > 0) {
			for (j = 0; j < i; j++) {
				if (sigismember(&oactl, siglist[j]) != 1) {
					test_failed = 1;
				}
			}

			for (j = i + 1; j < NUMSIGNALS; j++) {
				if (sigismember(&oactl, siglist[j]) != 0) {
					test_failed = 1;
				}
			}
		}
	}

	if (test_failed != 0) {
		printf("Old set is invalid.\n");
		pthread_exit((void *)-1);
	}

	printf
	    ("Test PASSED: oactl did contain all signals that were added to the signal mask.\n");
	pthread_exit(NULL);

	/* To please some compilers */
	return NULL;
}

int main(void)
{

	int *thread_return_value;

	pthread_t new_thread;

	if (pthread_create(&new_thread, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating new thread\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_join(new_thread, (void *)&thread_return_value) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if ((long)thread_return_value != 0) {
		if ((long)thread_return_value == 1) {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		} else if ((long)thread_return_value == -1) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		} else {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		}
	}

	return PTS_PASS;
}
