/*
* Copyright (c) 2003, Intel Corporation. All rights reserved.
* Created by:  salwan.searty REMOVE-THIS AT intel DOT com
* This file is licensed under the GPL license.  For the full content
* of this license, see the COPYING file at the top level of this
* source tree.

The resulting set shall be the the signal set pointed to by set

Steps:
1. Have main create a new thread and wait for its termination.
2. Inside the new thread, set up the signal mask such that it contains
 only SIGABRT (by passing SIG_SETMASK value to pthread_sigmask)
4. Raise SIGABRT, and make sure that the handler associated with it
 wasn't executed, otherwise fail.
5. Also make sure that SIGABRT is pending, otherwise fail.

* patch *
5b. Change mask to SIGUSR1 and check that the handler is called. This means
that SIG_SETMASK removed the old signal from the set.
* /patch *

6. Pass one of three return codes to the main() function:
 - A value of -1 if SIGABRT wasn't found pending or
   causes the handler to be executed.
 - A value of 0 if SIGABRT was in fact pending and the handler
   wasn't executed.
 - A value of 1 incase of any UNRESOLVED situation such as an
   unexpected function failure.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

static volatile int handler_called;

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
}

void *a_thread_func()
{

	struct sigaction act;
	sigset_t blocked_set, pending_set;
	sigemptyset(&blocked_set);
	sigaddset(&blocked_set, SIGABRT);

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGABRT, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void *)1);
	}

	if (pthread_sigmask(SIG_SETMASK, &blocked_set, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to use pthread_sigmask.\n");
		pthread_exit((void *)1);
	}

	if (raise(SIGABRT) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void *)1);
	}

	if (handler_called) {
		printf("FAIL: Signal was not blocked\n");
		pthread_exit((void *)-1);
	}

	if (sigpending(&pending_set) == -1) {
		perror("Unexpected error while attempting to use sigpending\n");
		pthread_exit((void *)1);
	}

	if (sigismember(&pending_set, SIGABRT) == -1) {
		perror("Unexpected error while attempting to use sigismember.");
		pthread_exit((void *)-1);
	}

	if (sigismember(&pending_set, SIGABRT) != 1) {
		perror("FAIL: sigismember did not return 1");
		pthread_exit((void *)1);
	}

	sigemptyset(&blocked_set);
	sigaddset(&blocked_set, SIGUSR1);

	if (pthread_sigmask(SIG_SETMASK, &blocked_set, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to use pthread_sigmask.");
		pthread_exit((void *)1);
	}

	sched_yield();

	if (!handler_called) {
		printf("FAIL: Old signal was not removed from mask.\n");
		pthread_exit((void *)-1);
	}

	pthread_exit(NULL);
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

	printf("Test PASSED\n");
	return PTS_PASS;
}
