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
	int child_pid, stat_loc, old_priority;

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

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
	param.sched_priority++;
	sched_setparam(child_pid, &param);

	if (sched_getparam(0, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	if (param.sched_priority == old_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("The priority have changed.\n");
		return PTS_FAIL;
	}
}
