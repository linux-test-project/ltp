#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "posixtest.h"

/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that if the user ID of the sending process doesn't match
 *  the user id of the receiving process, the kill function will fail
 *  with errno set to EPERM.
 *  1) Fork a child process.
 *     In the parent process,
 *     2)  Set the UID to 1.
 *     3)  Call kill for the pid of the child.
 *  In the child,
 *    3) Wait for signal from parent.
 *    4) If the signal is received, return -1 for failure.
 *  In the parent,
 *    5) Verify that the kill command failed with errno set to EPERM.
 *    6) If the kill command did not fail, send an abort signal to the child.
 *
 *  This test is only performed on one signal.  All other signals are
 *  considered to be in the same equivalence class.
 *
 */

#define SIGTOTEST SIGALRM

int main()
{
	int pid;
	int sig;
	sigset_t set;
	if (sigemptyset(&set) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaddset(&set, SIGTOTEST) == -1) {
		perror("Error calling sigaddset\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */

		if (0 == sigwait(&set, &sig)) {
			printf("child incorrectly received signal\n");
			return PTS_UNRESOLVED;
		}
		return PTS_UNRESOLVED;
	} else {
		/* parent here */

	        if (-1 == setuid(1)) {
			printf("Error setting user ID\n");
			return PTS_UNRESOLVED;
        	}

		sleep(1);
		if (0 == kill(pid, SIGTOTEST)) {
			printf("kill() incorrectly succeeded\n");
			kill(pid, SIGABRT);
			printf("Test FAILED\n");
			return PTS_FAIL;
		} else {
			if (EPERM == errno) {
				printf("Test PASSED\n");
				return PTS_PASS;
			} else {
				printf("kill failed; errno != EPERM\n");
				printf("Test FAILED\n");
				return PTS_FAIL;
			}
		}
	}

	printf("Should have exited from parent\n");
	printf("Test FAILED\n");
	return PTS_FAIL;
}

