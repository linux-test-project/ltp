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

 * This utility software allows to run any executable file with a timeout limit.
 * The syntax is:
 * $ ./t0 n exe arglist
 *  where n is the timeout duration in seconds,
 *        exe is the executable filename to run,
 *        arglist is the arguments to be passed to executable.
 *
 * The use of this utility is intended to be "transparent", which means
 * everything is as if
 * $ exe arglist
 *   had been called, and a call to "alarm(n)" had been added inside exe's main.
 *
 * SPECIAL CASE:
 * $ ./t0 0
 *  Here another arg is not required. This special case will return immediatly
 *  as if it has been timedout. This is useful to check a timeout return code value.
 *
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pid_t pid_to_monitor;

void sighandler(int sig)
{
	if (0 < pid_to_monitor) {
		if (kill(pid_to_monitor, SIGKILL) == -1) {
			perror("kill(.., SIGKILL) failed");
			abort();	/* Something's really screwed up if we get here. */
		}
		waitpid(pid_to_monitor, NULL, WNOHANG);
	}
	exit(SIGALRM + 128);
}

int main(int argc, char *argv[])
{
	int status, timeout;

	/* Special case: t0 0 */
	if (argc == 2 && (strncmp(argv[1], "0", 1) == 0)) {
		kill(getpid(), SIGALRM);
		exit(1);
	}

	/* General case */
	if (argc < 3) {
		printf("\nUsage: \n");
		printf("  $ %s n exe arglist\n", argv[0]);
		printf("  $ %s 0\n", argv[0]);
		printf("\nWhere:\n");
		printf("  n       is the timeout duration in seconds,\n");
		printf("  exe     is the executable filename to run,\n");
		printf
		    ("  arglist is the arguments to be passed to executable.\n\n");
		printf
		    ("  The second use case will emulate an immediate timeout.\n\n");
		exit(1);
	}

	timeout = atoi(argv[1]);
	if (timeout < 1) {
		fprintf(stderr,
			"Invalid timeout value \"%s\". Timeout must be a positive integer.\n",
			argv[1]);
		exit(1);
	}

	if (signal(SIGALRM, sighandler) == SIG_ERR) {
		perror("signal failed");
		exit(1);
	}

	alarm(timeout);

	switch (pid_to_monitor = fork()) {
	case -1:
		perror("fork failed");
		exit(1);
	case 0:
		setpgid(0, 0);
		execvp(argv[2], &argv[2]);
		perror("execvp failed");
		exit(1);
	default:

		for (;;) {
			if (waitpid(pid_to_monitor, &status, 0) ==
			    pid_to_monitor)
				break;
			else if (errno == EINTR) {
				perror("waitpid failed");
				exit(1);
			}
		}
		/* Relay the child's status back to run-tests.sh */
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
		else if (WIFSIGNALED(status))
			exit(WTERMSIG(status) + 128);

	}

	exit(1);
}
