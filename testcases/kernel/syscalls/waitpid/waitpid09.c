// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * case 0:
 *        waitpid(pid, WNOHANG) should return 0 if there is a running child
 * case 1:
 *        waitpid(pid, WNOHANG) should return the pid of the child if
 *        the child has exited
 * case 2:
 *        waitpid(-1, 0) should return -1 with ECHILD if
 *        there are no children to wait for.
 * case 3:
 *        waitpid(-1, WNOHANG) should return -1 with ECHILD if
 *        there are no children to wait for.
 */

#define _GNU_SOURCE 1
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"

static void cleanup_pid(pid_t pid)
{
	if (pid > 0) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
}

static void case0(void)
{
	pid_t pid, ret;
	int status;

	pid = SAFE_FORK();
	if (pid == 0) {
		TST_CHECKPOINT_WAIT(0);

		exit(0);
	}

	for (;;) {
		ret = waitpid(pid, &status, WNOHANG);

		if ((ret == -1) && (errno == EINTR))
			continue;

		if (ret == 0)
			break;

		tst_res(TFAIL, "waitpid(WNOHANG) returned %d, expected 0",
			ret);
		cleanup_pid(pid);
		return;
	}

	TST_CHECKPOINT_WAKE(0);
	SAFE_WAITPID(pid, NULL, 0);

	tst_res(TPASS, "waitpid(pid, WNOHANG) = 0 for a running child");
}

static void case1(void)
{
	pid_t pid, ret;
	int status;

	pid = SAFE_FORK();
	if (pid == 0)
		exit(0);

	for (;;) {
		ret = waitpid(pid, &status, WNOHANG);

		if ((ret == -1) && (errno == EINTR))
			continue;
		if (ret == 0)
			continue;

		if (ret == pid)
			break;

		tst_res(TFAIL, "waitpid(WNOHANG) returned %d, expected %d",
			ret, pid);
		cleanup_pid(pid);
		return;
	}

	if (!WIFEXITED(status)) {
		tst_res(TFAIL, "Child exited abnormally");
		return;
	}

	if (WEXITSTATUS(status) != 0) {
		tst_res(TFAIL, "Child exited with %d, expected 0",
			WEXITSTATUS(status));
		return;
	}

	tst_res(TPASS, "waitpid(pid, WNOHANG) = pid for an exited child");
}

static void case2(void)
{
	pid_t ret;
	int status;

	ret = waitpid(-1, &status, 0);

	if (ret != -1) {
		tst_res(TFAIL, "Expected -1, got %d", ret);
		return;
	}
	if (errno != ECHILD) {
		tst_res(TFAIL, "Expected %s, got %s",
			tst_strerrno(ECHILD), tst_strerrno(errno));
		return;
	}

	tst_res(TPASS, "waitpid(-1, 0) = -1 with ECHILD if no children");
}

static void case3(void)
{
	pid_t ret;
	int status;

	ret = waitpid(-1, &status, WNOHANG);
	if (ret != -1) {
		tst_res(TFAIL, "WNOHANG: Expected -1, got %d", ret);
		return;
	}
	if (errno != ECHILD) {
		tst_res(TFAIL, "WNOHANG: Expected %s, got %s",
			tst_strerrno(ECHILD), tst_strerrno(errno));
		return;
	}

	tst_res(TPASS, "waitpid(-1, WNOHANG) = -1 with ECHILD if no children");
}

static void (*tests[])(void) = { case0, case1, case2, case3 };

static void waitpid09_test(unsigned int id)
{
	tests[id]();
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test = waitpid09_test,
	.tcnt = ARRAY_SIZE(tests),
};
