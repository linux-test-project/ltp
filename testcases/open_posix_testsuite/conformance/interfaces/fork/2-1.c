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

* This sample test aims to check the following assertion:
*
* The new process is a copy of the original one -- with few exceptions.

* The steps are:
* -> set up some data in process memory space
* -> create a new process
* -> check in this new process that the memory space was copied.
*
* We check that:
* -> a structure object is copied.
* -> a malloc'ed memory block is copied and can be freed in child.
* -> the environment is copied
* -> signal handlers are copied

* The test fails if a difference is detected.

*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include <signal.h>

#include "posixtest.h"
#include "mem_pattern.h"

struct test_struct {
	char one;
	short two;
	int three;
	void *four;
};

void handler(int sig)
{
	(void)sig;
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;
	void *malloced;
	struct sigaction sa_ori, sa_child;
	struct test_struct mystruct = { 1, 2, 3, (void *)4 };
	size_t page_size = sysconf(_SC_PAGESIZE);

	malloced = malloc(page_size);

	if (malloced == NULL) {
		perror("malloc() failed");
		return PTS_UNRESOLVED;
	}

	fill_mem(malloced, page_size);

	/* Initialize an environment variable */
	ret = setenv("OPTS_FORK_TC", "2-1.c", 1);

	if (ret != 0) {
		perror("setenv() failed");
		return PTS_UNRESOLVED;
	}

	/* Initialize the signal handler */
	sa_ori.sa_handler = handler;

	ret = sigemptyset(&sa_ori.sa_mask);

	if (ret != 0) {
		perror("sigemptyset() failed");
		return PTS_UNRESOLVED;
	}

	ret = sigaddset(&sa_ori.sa_mask, SIGUSR2);

	if (ret != 0) {
		perror("sigaddset() failed");
		return PTS_UNRESOLVED;
	}

	sa_ori.sa_flags = SA_NOCLDSTOP;
	ret = sigaction(SIGUSR1, &sa_ori, NULL);

	if (ret != 0) {
		perror("sigaction() failed");
		return PTS_UNRESOLVED;
	}

	/* Create the child */
	child = fork();

	if (child == -1) {
		perror("fork() failed");
		return PTS_UNRESOLVED;
	}

	/* child */
	if (child == 0) {
		/* Check the struct was copied */
		if ((mystruct.one != 1) || (mystruct.two != 2) ||
		    (mystruct.three != 3) || (mystruct.four != (void *)4)) {
			printf("On-the-stack structure not copied correctly\n");
			return PTS_FAIL;
		}

		/* Check the malloc'ed memory is copied */
		if (check_mem(malloced, page_size)) {
			printf("Allocated page not copied correctly\n");
			return PTS_FAIL;
		}

		/* Free the memory -- this should suceed */
		free(malloced);

		/* Check the env variable */
		if (strncmp("2-1.c", getenv("OPTS_FORK_TC"), 6) != 0) {
			printf("Enviroment variable not copied correctly\n");
			return PTS_FAIL;
		}

		/* Check the signal handler stuff */
		ret = sigaction(SIGUSR1, NULL, &sa_child);

		if (ret != 0) {
			perror("sigaction() failed in child");
			return PTS_UNRESOLVED;
		}

		if (sa_child.sa_handler != handler) {
			printf("Signal handler function is different\n");
			return PTS_FAIL;
		}

		ret = sigismember(&sa_child.sa_mask, SIGUSR2);

		if (ret == 0) {
			printf("Signal handler mask is different\n");
			return PTS_FAIL;
		}

		if (ret != 1) {
			printf("Unexpected return code from sigismember\n");
			return PTS_UNRESOLVED;
		}

		if (((sa_child.sa_flags & SA_NOCLDSTOP) != SA_NOCLDSTOP)
#ifndef WITHOUT_XOPEN
		    || ((sa_child.sa_flags & SA_ONSTACK) != 0)
		    || ((sa_child.sa_flags & SA_RESETHAND) != 0)
		    || ((sa_child.sa_flags & SA_RESTART) != 0)
		    || ((sa_child.sa_flags & SA_SIGINFO) != 0)
		    || ((sa_child.sa_flags & SA_NOCLDWAIT) != 0)
		    || ((sa_child.sa_flags & SA_NODEFER) != 0)
#endif
		    ) {
			printf("The sigaction flags are different\n");
			return PTS_FAIL;
		}

		return PTS_PASS;
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		printf("Waitpid returned wrong PID\n");
		return PTS_UNRESOLVED;
	}

	if (!WIFEXITED(status)) {
		printf("Child exited abnormally\n");
		return PTS_UNRESOLVED;
	}

	if (WEXITSTATUS(status) == PTS_PASS) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return WEXITSTATUS(status);
}
