/*
  Test case for assertion #1 of the WIFEXITED macro defined in sys/wait.h
  by forking a new process, having the child SIGABRT, and verifying
  WIFEXITED returns zero.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	int s;

	switch (fork()) {
	case 0:
		/* child */
		sleep(5);

		raise(SIGABRT);

		/* shouldn't get here */
		exit(0);
	case -1:
		perror("fork failed");
		break;
	default:

		/* parent */
		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			exit(PTS_UNRESOLVED);
		}

		if (!WIFEXITED(s)) {
			printf("Test PASSED\n");
			exit(PTS_PASS);
		}
		break;
	}

	printf("Test FAILED\n");
	exit(PTS_FAIL);
}
