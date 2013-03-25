/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that sched_getscheduler() returns -1 on failure.
 *
 * The test create a child process which exit immediately and call
 * sched_getscheduler with the pid of defunct child.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "posixtest.h"

int main(void)
{

	int result = -1, child_pid;
	int stat_loc;

	/* Create a child process which exit immediately */
	child_pid = fork();
	if (child_pid == -1) {
		perror("fork failed");
		return PTS_UNRESOLVED;
	} else if (child_pid == 0) {
		exit(0);
	}

	/* Wait for the child process to exit */
	if (wait(&stat_loc) == -1) {
		perror("wait failed");
		return PTS_UNRESOLVED;
	}

	/* Assume the pid is not yet reatributed to an other process */
	result = sched_getscheduler(child_pid);

	if (result == -1) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (errno != ESRCH) {
		perror("ESRCH is not returned");
		return PTS_FAIL;
	}

	if (result != -1) {
		printf("Returned code is not -1.\n");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;
	}

}
