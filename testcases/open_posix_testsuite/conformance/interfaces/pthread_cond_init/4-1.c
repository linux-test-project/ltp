/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * This sample test aims to check the following assertion:
 * The function fails and returns ENOMEM if there is not enough memory.
 *
 * The steps are:
 * * Fork
 * * New process sets its memory resource limit to a minimum value, then
 *  -> Allocate all the available memory
 *  -> call pthread_cond_init()
 *  -> free the memory
 *  -> Checks that pthread_cond_init() returned 0 or ENOMEM.
 * * Parent process waits for the child.
 *
 *
 *  Updated coding style - Peter W. Morreale <pmorreale AT novell DOT com>
 *  Date:  31/05/2011
 */


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <stdarg.h>
#include <posixtest.h>

#define ERR_MSG(f, rc) printf("Failed: func: %s rc: %s (%u)\n", \
			f, strerror(rc), rc);

/* Max memory for child is 1MB */
#define MAX_MEM	((1<<20))

/*
 * The child process - consume memory and exit with
 * the test exit code
 */
static void child(void)
{
	void *curr;
	void *prev = NULL;
	struct rlimit rl;
	pthread_cond_t cond;
	int rc;

	/* Limit the process memory to a small value (8 for example). */
	rl.rlim_max = MAX_MEM;
	rl.rlim_cur = MAX_MEM;
	rc = setrlimit(RLIMIT_DATA, &rl);
	rc |= setrlimit(RLIMIT_AS, &rl);
	if (rc < 0) {
		ERR_MSG("setrlimit()", errno);
		exit(PTS_UNRESOLVED);
	}

	/*
	 * Consume all memory we can
	 * It's importamt to use the malloc() return value in a
	 * meaningful way to bypass potential compiler optimisations.
	 */
	while ((curr = malloc(sizeof(void *)))) {
		*(void **)curr = prev;
		prev = curr;
	}
	if (errno != ENOMEM) {
		ERR_MSG("malloc()", errno);
		exit(PTS_UNRESOLVED);
	}

	/*
	 * Exit from this child process with the return code
	 * Note that pthread_cond_init can succeed in low-mem
	 * conditions
	 */
	rc = pthread_cond_init(&cond, NULL);
	if (!rc || rc == ENOMEM) {
		rc = PTS_PASS;
	} else {
		ERR_MSG("pthread_cond_init()", rc);
		rc = PTS_FAIL;
	}

	exit(rc);
}

int main(void)
{
	pid_t pid;
	int child_status;
	int status = PTS_UNRESOLVED;

	pid = fork();
	if (pid < 0) {
		ERR_MSG("fork()", errno);
		goto done;
	}

	/* Child executes and exits here */
	if (pid == 0)
		child();

	/* Parent waits for child to complete */
	if (pid != waitpid(pid, &child_status, 0)) {
		ERR_MSG("waitpid()", errno);
		goto done;
	}

	if (WIFSIGNALED(child_status)) {
		printf("Failed: child received signal: %u\n",
		       WTERMSIG(child_status));
		goto done;
	}

	if (WIFEXITED(child_status))
		status = WEXITSTATUS(child_status);

	if (status == PTS_PASS)
		printf("Test PASSED\n");

done:
	return status;
}
