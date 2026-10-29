/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test to see if timer_gettime() sets errno==EINVAL for an invalid
 * timerid. Since POSIX 2016 Edition is this behavior undefined, so
 * returning -1 with EINVAL, succeeding, or crashing (older glibc
 * dereferences timerid as a pointer) are all accepted outcomes.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include "posixtest.h"

#define BOGUSTID 9999

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return PTS_UNRESOLVED;
	}

	if (pid == 0) {
		struct itimerspec its;
		timer_t tid = (timer_t) BOGUSTID;

		if (timer_gettime(tid, &its) == -1) {
			if (errno == EINVAL)
				_exit(PTS_PASS);
			printf("timer_gettime() returned -1, but errno=%d (%s), not EINVAL\n", errno, strerror(errno));
			fflush(stdout);
			_exit(PTS_FAIL);
		}
		_exit(PTS_PASS);
	}

	if (waitpid(pid, &status, 0) == -1) {
		perror("waitpid");
		return PTS_UNRESOLVED;
	}

	if ((WIFSIGNALED(status) && (WTERMSIG(status) == SIGSEGV || WTERMSIG(status) == SIGBUS)) ||
		(WIFEXITED(status) && WEXITSTATUS(status) == PTS_PASS)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
