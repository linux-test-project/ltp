/*
  Test case for assertion #1 of the WIFEXITED macro defined in sys/wait.h
  by forking a new process, having the process return 0, and verifying 
  WIFEXITED returns a non-negative value.
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
	if (fork() == 0) {
		/* child */
		return 0;
	} else {
		int s; 

		/* parent */
		if (wait(&s) == -1) {
			perror("Unexpected error while setting up test "
			       "pre-conditions");
			return -1;
		}

		if (WIFEXITED(s)) {
			printf("Test PASSED\n");
			return 0;
		}
	}

	printf("Test FAILED\n");
	return -1;	
}

