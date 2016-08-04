/*
 * Copyright (c) 2016 Linux Test Project
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

#ifndef WAITPID_COMMON_H__
#define WAITPID_COMMON_H__

#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"

#define	MAXKIDS	8

static pid_t *fork_kid_pid;
static pid_t child_1_pid;

static void do_child_1(void);

static void waitpid_setup(void)
{
	fork_kid_pid = SAFE_MMAP(NULL, sizeof(*fork_kid_pid) * MAXKIDS,
				 PROT_READ | PROT_WRITE,
				 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void waitpid_cleanup(void)
{
	int i;

	for (i = 0; i < MAXKIDS; i++) {
		if (fork_kid_pid[i] > 0)
			kill(fork_kid_pid[i], SIGKILL);
	}

	if (child_1_pid > 0)
		kill(child_1_pid, SIGKILL);

	munmap(fork_kid_pid, sizeof(*fork_kid_pid) * MAXKIDS);
}

static void waitpid_test(void)
{
	child_1_pid = SAFE_FORK();
	if (child_1_pid == 0) {
		do_child_1();
	} else {
		tst_reap_children();
		child_1_pid = 0;
	}
}

static void do_exit(int stop)
{
	TST_CHECKPOINT_WAIT(0);

	if (stop)
		kill(getpid(), SIGSTOP);

	exit(3);
}

static int reap_children(pid_t wp_pid, int wp_opts, pid_t *children, int len)
{
	pid_t pid;
	int i;
	int status;

	for (;;) {
		pid = waitpid(wp_pid, &status, wp_opts);

		if (pid == -1) {
			if (errno == EINTR)
				continue;

			break;
		}

		if (pid == 0) {
			if (wp_opts & WNOHANG)
				continue;

			tst_res(TFAIL, "waitpid() returned 0 unexpectedly");
			return -1;
		}

		for (i = 0; i < len; i++) {
			if (pid == children[i]) {
				children[i] = 0;
				break;
			}
		}

		if (i == len) {
			tst_res(TFAIL, "Pid %d not found", pid);
			return -1;
		}

		if (!WIFEXITED(status)) {
			tst_res(TFAIL, "Pid %d exited abnormally", pid);
			return -1;
		}

		if (WEXITSTATUS(status) != 3) {
			tst_res(TFAIL, "Pid %d exited with %d, expected 3",
				pid, WEXITSTATUS(status));
			return -1;
		}
	}

	for (i = 0; i < len; i++) {
		if (children[i]) {
			tst_res(TFAIL, "Pid %d not reaped", children[i]);
			return -1;
		}
	}

	return 0;
}

#endif /* WAITPID_COMMON_H__ */
