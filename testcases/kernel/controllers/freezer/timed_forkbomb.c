/* Copyright (c) International Business Machines  Corp., 2009
 * Author: Matt Helsley <matthltc@us.ibm.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char **argv)
{
	struct timeval start, now;
	pid_t child_root;
	unsigned int duration = 1, num_tasks = 0;

	if (argc > 1) {
		int i;

		for (i = 0; i < argc; i++) {
			if (strcmp(argv[i], "--help") == 0) {
				printf("%s [duration]\n", argv[0]);
				exit(EXIT_SUCCESS);
			}

			if (sscanf(argv[i], "%u", &duration) == 1) {
				printf("%u second duration\n", duration);
				break;
			}
		}
	}

	/*
	 * Wait until we're told to go. This allows the driver script to
	 * configure the test system (e.g. by putting this task in a
	 * suitable cgroup) before the timed forkbomb begins.
	 */
	{
		char *word;
		while (scanf("%as", &word) < 1) {
		}
		free(word);
	}
	if (gettimeofday(&start, NULL))
		exit(EXIT_FAILURE);
	child_root = fork();
	if (child_root == -1)
		exit(EXIT_FAILURE);
/*	if (child_root == 0)
		signal(SIGCHLD, SIG_IGN);*/
	do {
		if (child_root == 0) {
/*			signal(SIGCHLD, SIG_IGN);*/
			if (fork() == -1)
				break;
		} else {
			if (wait(NULL) > 0)
				num_tasks++;
		}
		if (gettimeofday(&now, NULL) && (errno != EINTR))
			break;
	} while ((now.tv_sec - start.tv_sec) <= duration);

	if (child_root != 0)
		printf("Forked %d tasks\n", num_tasks);
	exit(EXIT_SUCCESS);
}
