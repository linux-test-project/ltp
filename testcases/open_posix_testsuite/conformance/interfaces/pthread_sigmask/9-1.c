/*
  Test case for assertion #1 of the sigaction system call that shows
  sigaction (when used with a non-null act pointer) changes the action
  for a signal.

  Steps:
  1. Use sigaction to setup a signal handler for SIGABRT
  2. Add SIGABRT to the signal mask.
  3. Raise SIGABRT. Now, SIGABRT is pending.
  4. Call pthread_sigmask() again. Verify that global variable 
     pthread_sigmask_return_val is not set to anything other than 
     it's initial value (which is 1), while we're still inside 
     the signal handler code.
  5. Once pthread_sigmask() returns, verify that it returns a zero, 
     and verify that the global handler_called variable has been set to 1; 
  6. If we manage to verify both steps 4 and 5, then we've 
     proved that signal was delivered before pthread_sigmask() returned.

*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int handler_called = 0;
int pthread_sigmask_return_val = 1; /* some value that's not a 1 or 0 */

void handler(int signo)
{
	handler_called = 1;
	if (pthread_sigmask_return_val != 1) {
		printf("FAIL: pthread_sigmask() returned before signal was delivered.\n");
		exit(PTS_FAIL);
	}
}

void *a_thread_func()
{
	struct sigaction act;
	sigset_t blocked_set1;
	sigemptyset(&blocked_set1);
	sigaddset(&blocked_set1, SIGABRT);

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGABRT,  &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void*)1);
	}

	if (pthread_sigmask(SIG_SETMASK, &blocked_set1, NULL) == -1) {
		perror("Unexpected error while attempting to use pthread_sigmask.\n");
		pthread_exit((void*)1);
	}

	if ((raise(SIGABRT) == -1)) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		pthread_exit((void*)1);
	}

	pthread_sigmask_return_val = pthread_sigmask(SIG_UNBLOCK, &blocked_set1, NULL);

	if (pthread_sigmask_return_val != 0) {
		perror("Unexpected error while attempting to use pthread_sigmask.\n");
		pthread_exit((void*)1);
	}
	
	if (handler_called != 1) {
		perror("Handler wasn't called, implying signal was not delivered.\n");
		pthread_exit((void*)1);
	}	

	printf("Test PASSED: signal was delivered before the call to pthread_sigmask returned.\n");
	pthread_exit((void*)0);

        /* To please some compilers */
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

