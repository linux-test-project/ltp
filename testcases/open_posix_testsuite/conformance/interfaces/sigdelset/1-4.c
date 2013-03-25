/*
   Test the error condition of calling sigdelset() twice for the same
   signo.
   Test steps:
   1)  Initialize an empty signal set.
   2)  Add the SIGALRM signal to the empty signal set and verify.
   3)  Remove the SIGALRM signal from the signal set and verify removed.
   4)  Attempt to remove again and verify signal is still removed.
 */
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if (sigemptyset(&signalset) == -1) {
		perror("sigemptyset failed -- test aborted");
		return PTS_UNRESOLVED;
	}
	if (sigaddset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 0) {
			printf("sigaddset returned, sigismember failed\n");
			return PTS_UNRESOLVED;
		}
	} else {
		printf("sigaddset did not successfully add signal\n");
		return PTS_UNRESOLVED;
	}

	if (sigdelset(&signalset, SIGALRM) == 0) {
		if (sigismember(&signalset, SIGALRM) == 1) {
			printf("Signal is still in signal set.\n");
			return PTS_FAIL;
		}
	} else {
		printf("sigdelset() failed\n");
		return PTS_FAIL;
	}

	sigdelset(&signalset, SIGALRM);
	if (sigismember(&signalset, SIGALRM) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
