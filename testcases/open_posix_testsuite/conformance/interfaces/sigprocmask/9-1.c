/*
  Test case for assertion #1 of the sigaction system call that shows
  sigaction (when used with a non-null act pointer) changes the action
  for a signal.

  Steps:
  1. Use sigaction to setup a signal handler for SIGABRT
  2. Add SIGABRT to the signal mask.
  3. Raise SIGABRT. Now, SIGABRT is pending.
  4. Call sigprocmask() again. Verify that global variable
     sigprocmask_return_val is not set to anything other than
     it's initial value (which is 1), while we're still inside
     the signal handler code.
  5. Once sigprocmask() returns, verify that it returns a zero, and
     verify that the global handler_called variable has been set to 1;
  6. If we manage to verify both steps 4 and 5, then we've
     proved that signal was delivered before sigprocmask() returned.

*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

static volatile int handler_called;
int sigprocmask_return_val = 1;	/* some value that's not a 1 or 0 */

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	handler_called = 1;
	if (sigprocmask_return_val != 1) {
		printf
		    ("FAIL: sigprocmask() returned before signal was delivered.\n");
		exit(PTS_FAIL);
	}
}

int main(void)
{
	struct sigaction act;
	sigset_t blocked_set1;
	sigemptyset(&blocked_set1);
	sigaddset(&blocked_set1, SIGABRT);

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGABRT, &act, 0) == -1) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	if (sigprocmask(SIG_SETMASK, &blocked_set1, NULL) == -1) {
		perror
		    ("Unexpected error while attempting to use sigprocmask.\n");
		return PTS_UNRESOLVED;
	}

	if ((raise(SIGABRT) == -1)) {
		perror("Unexpected error while attempting to setup test "
		       "pre-conditions");
		return PTS_UNRESOLVED;
	}

	sigprocmask_return_val = sigprocmask(SIG_UNBLOCK, &blocked_set1, NULL);

	if (sigprocmask_return_val != 0) {
		perror
		    ("Unexpected error while attempting to use sigprocmask.\n");
		return PTS_UNRESOLVED;
	}

	if (handler_called != 1) {
		perror
		    ("Handler wasn't called, implying signal was not delivered.\n");
		return PTS_UNRESOLVED;
	}

	printf
	    ("Test PASSED: signal was delivered before the call to sigprocmask returned.\n");
	return PTS_PASS;
}
