/*
 * Copyright (c) International Business Machines  Corp., 2012
 * Copyright (c) Linux Test Project, 2012
 *
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "process_vm_readv02";
int TST_TOTAL = 1;

static char *tst_string = "THIS IS A TEST";
static int len;
static int pipe_fd[2];
static pid_t pids[2];

static void child_alloc(void);
static void child_invoke(void);
static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc, status;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		len = strlen(tst_string);

		SAFE_PIPE(cleanup, pipe_fd);

		/* the start of child_alloc and child_invoke is already
		 * synchronized via pipe */
		pids[0] = fork();
		switch (pids[0]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #0");
		case 0:
			child_alloc();
			exit(0);
		}

		pids[1] = fork();
		switch (pids[1]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #1");
		case 0:
			child_invoke();
			exit(0);
		}

		/* wait until child_invoke reads from child_alloc's VM */
		SAFE_WAITPID(cleanup, pids[1], &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child 1 returns %d", status);

		/* child_alloc is free to exit now */
		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

		SAFE_WAITPID(cleanup, pids[0], &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child 0 returns %d", status);
	}

	cleanup();
	tst_exit();
}

static void child_alloc(void)
{
	char *foo;
	char buf[BUFSIZ];

	foo = SAFE_MALLOC(tst_exit, len + 1);
	strncpy(foo, tst_string, len);
	foo[len] = '\0';
	tst_resm(TINFO, "child 0: memory allocated and initialized.");

	/* passing addr of string "foo" via pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	snprintf(buf, BUFSIZ, "%p", foo);
	SAFE_WRITE(tst_exit, 1, pipe_fd[1], buf, strlen(buf) + 1);
	SAFE_CLOSE(tst_exit, pipe_fd[1]);

	/* wait until child_invoke is done reading from our VM */
	TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);
}

static void child_invoke(void)
{
	char *lp, *rp;
	char buf[BUFSIZ];
	struct iovec local, remote;

	/* get addr from pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[1]);
	SAFE_READ(tst_exit, 0, pipe_fd[0], buf, BUFSIZ);
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	if (sscanf(buf, "%p", &rp) != 1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sscanf");

	lp = SAFE_MALLOC(tst_exit, len + 1);
	local.iov_base = lp;
	local.iov_len = len;
	remote.iov_base = rp;
	remote.iov_len = len;

	tst_resm(TINFO, "child 1: reading string from same memory location.");
	TEST(tst_syscall(__NR_process_vm_readv, pids[0],
			 &local, 1UL, &remote, 1UL, 0UL));
	if (TEST_RETURN != len)
		tst_brkm(TFAIL | TTERRNO, tst_exit, "process_vm_readv");
	if (strncmp(lp, tst_string, len) != 0)
		tst_brkm(TFAIL, tst_exit, "child 1: expected string: %s, "
			 "received string: %256s", tst_string, lp);
	else
		tst_resm(TPASS, "expected string received.");
}

static void setup(void)
{
	tst_require_root();

	/* Just a sanity check of the existence of syscall */
	tst_syscall(__NR_process_vm_readv, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	tst_tmpdir();
	TST_CHECKPOINT_INIT(cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
