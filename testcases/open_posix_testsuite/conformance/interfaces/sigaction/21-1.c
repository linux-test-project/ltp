/*
  Test assertion #21 by verifying that adding a sigaction for SIGCHLD
  with the SA_NOCLDWAIT bit set in sigaction.sa_flags will result in
  a child process not transforming into a zombie after death.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Caught SIGCHLD\n");
}

int main(void)
{

	/* Make sure this flag is supported. */
#ifndef SA_NOCLDWAIT
	fprintf(stderr, "SA_NOCLWAIT flag is not available for testing\n");
	return PTS_UNSUPPORTED;
#endif

	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, 0);

	if (fork() == 0) {
		/* child */
		return 0;
	} else {
		/* parent */
		int s;
		if (wait(&s) == -1 && errno == ECHILD) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
