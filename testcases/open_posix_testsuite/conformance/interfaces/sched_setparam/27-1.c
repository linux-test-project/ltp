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
 * Test that sched_setparam() sets errno == ESRCH when no process can be found
 * corresponding to that specified by pid.
 *
 * The test create a child process which exit immediately and call
 * sched_setparam with the pid of defunct child.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "posixtest.h"

int main(void)
{
	struct sched_param param;
	int result, child_pid, stat_loc;

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	/* Create a child process which exit immediately */
	child_pid = fork();
	if (child_pid == -1) {
		perror("An error occurs when calling fork()");
		return PTS_UNRESOLVED;
	} else if (child_pid == 0) {
		exit(0);
	}

	/* Wait for the child process to exit */
	if (wait(&stat_loc) == -1) {
		perror("An error occurs when calling wait()");
		return PTS_UNRESOLVED;
	}

	/* Assume the pid is not yet reatributed to an other process */
	result = sched_setparam(child_pid, &param);

	if (result == -1 && errno == ESRCH) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("The returned code is not -1.\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("This process does not have the permission to invoke sched_setparam().\nTry to launch this test as root\n");
		return PTS_UNRESOLVED;
	} else {
		perror("errno is not ESRCH");
		return PTS_FAIL;
	}
}
