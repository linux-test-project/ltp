/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *   Date:    11/08/2011
 */

#include "affinity.h"

#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"


#define ERR_LOG(l, rc)   printf("Failed: %s rc: %d errno: %s\n", \
					l, rc, strerror(errno))

static int child_busy(int fd)
{
	int rc;

	/* suicide if sched_yield fails */
	alarm(4);

	/* Tell the parent we're ready */
	if (write(fd, "go", 2) == -1) {
		perror("write");
		exit(PTS_UNRESOLVED);
	}

	for (;;) {
		rc = sched_yield();
		if (rc) {
			ERR_LOG("child: sched_yield", rc);
			exit(PTS_FAIL);
		}
	}

	/* should not get here */
	exit(PTS_UNRESOLVED);
}

int main(void)
{
	int pid;
	int rc;
	int pfd[2];
	int status = PTS_UNRESOLVED;
	int s;
	struct sched_param sp;
	char buf[8];

	/* Set up a pipe, for synching.  */
	rc = pipe(pfd);
	if (rc) {
		ERR_LOG("pipe", rc);
		return status;
	}

	/* get in FIFO */
	sp.sched_priority = sched_get_priority_min(SCHED_FIFO);
	rc = sched_setscheduler(getpid(), SCHED_FIFO, &sp);
	if (rc) {
		ERR_LOG("sched_setscheduler", rc);
		return status;
	}

	/* Must only use a single CPU */
	rc = set_affinity_single();
	if (rc) {
		ERR_LOG("set_affinity_single", rc);
		return status;
	}

	pid = fork();
	if (pid == 0)
		child_busy(pfd[1]);

	if (pid < 0) {
		ERR_LOG("fork", rc);
		return status;
	}

	/* wait for child */
	rc = read(pfd[0], buf, sizeof(buf));
	if (rc != 2) {
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
		ERR_LOG("read", rc);
		return status;
	}

	/* Can only get here if sched_yield works. */
	kill(pid, SIGTERM);
	waitpid(pid, &s, 0);

	status = PTS_PASS;
	if (WIFSIGNALED(s)) {
		s = WTERMSIG(s);
		if (s != SIGTERM) {
			printf("Failed: kill signal: %d, should be: %d\n",
			       s, SIGTERM);
			status = PTS_FAIL;
		}
	} else if (WIFEXITED(s)) {
		printf("Failed: child prematurely exited with: %d\n",
		       WEXITSTATUS(s));
		status = PTS_FAIL;
	}

	if (status == PTS_PASS)
		printf("Test PASSED\n");

	return status;
}
