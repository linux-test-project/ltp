/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 The resulting set shall be the union of the current set and the signal 
 set pointed to by set, if the value of the argument how is SIG_BLOCK.

 Steps:
 1. Have main create a new thread and wait for its termination.
 2. Inside the new thread, set up the signal mask such that it contains 
    only SIGABRT.
 3. Also inside the new thread, using the SIG_BLOCK as the value to 
    pthread_sigmask's first parameter, add SIGALRM. Now both signals 
    should be in the signal mask of the new thread.
 4. Raise both signals make sure that the handler associated with these
    signals wasn't executed.
 5. Also make sure that both signals are pending.
 6. Pass one of three return codes to the main() function:
    - A value of -1 if one of the two signals wasn't found pending or 
      causes the handler to be executed.
    - A value of 0 if both signals were infact pending and the handler 
      wasn't executed.
    - A value of 1 incase of any UNRESOLVED situation such as an 
      unexpected function failure.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "posixtest.h"

int handler_called = 0;

void handler(int signo)
{
	handler_called = 1;
}


void *a_thread_func()
{
	struct sigaction act;
	sigset_t blocked_set1, blocked_set2, pending_set;
	sigemptyset(&blocked_set1);
	sigemptyset(&blocked_set2);
	sigaddset(&blocked_set1, SIGABRT);
	sigaddset(&blocked_set2, SIGALRM);

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGABRT,  &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void*)1);
	}

	if (sigaction(SIGALRM,  &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void*)1);
	}

	if (pthread_sigmask(SIG_SETMASK, &blocked_set1, NULL) == -1) {
		perror("Unexpected error while attempting to use pthread_sigmask.\n");
		pthread_exit((void*)1);
	}

	if (pthread_sigmask(SIG_BLOCK, &blocked_set2, NULL) == -1) {
		perror("Unexpected error while attempting to use pthread_sigmask.\n");
		pthread_exit((void*)1);
	}

	if ((raise(SIGABRT) == -1) | (raise(SIGALRM) == -1)) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void*)1);
	}

	if (handler_called) {
		printf("FAIL: Signal was not blocked\n");
		pthread_exit((void*)-1);
	}

	if (sigpending(&pending_set) == -1) {
		perror("Unexpected error while attempting to use sigpending\n");
		pthread_exit((void*)1);
	}

	if ((sigismember(&pending_set, SIGABRT) != 1) | (sigismember(&pending_set, SIGALRM) != 1)) {
		perror("FAIL: sigismember did not return 1\n");
		pthread_exit((void*)-1);
	}

	pthread_exit((void*)0);
	return NULL;
}

int main() {

	int *thread_return_value;

	pthread_t new_thread;

	if (pthread_create(&new_thread, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating new thread\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_join(new_thread, (void*)&thread_return_value) != 0) {
                perror("Error in pthread_join()\n");
                return PTS_UNRESOLVED;
        }

	if ((long)thread_return_value != 0) {
		if ((long)thread_return_value == 1) {
			printf ("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		}
		else if ((long)thread_return_value == -1) {
			printf ("Test FAILED\n");
			return PTS_FAIL;
		}
		else {
			printf ("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;			
		}
	}
	return PTS_PASS;
}
