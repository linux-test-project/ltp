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
#include <limits.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "process_vm_writev02";
int TST_TOTAL = 1;

#define PADDING_SIZE 10
#define DEFAULT_CHAR 53

static int sflag;
static char *sz_opt;
static option_t options[] = {
	{"s:", &sflag, &sz_opt},
	{NULL, NULL, NULL}
};

static long bufsz;
static int pipe_fd[2];
static pid_t pids[2];

static void child_init_and_verify(void);
static void child_write(void);
static void setup(void);
static void cleanup(void);
static void help(void);

int main(int argc, char **argv)
{
	int lc, status;

	tst_parse_opts(argc, argv, options, &help);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_PIPE(cleanup, pipe_fd);

		/* the start of child_init_and_verify and child_write is
		 * already synchronized via pipe */
		pids[0] = fork();
		switch (pids[0]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #0");
		case 0:
			child_init_and_verify();
			exit(0);
		default:
			break;
		}

		pids[1] = fork();
		switch (pids[1]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #1");
		case 0:
			child_write();
			exit(0);
		}

		/* wait until child_write writes into
		 * child_init_and_verify's VM */
		SAFE_WAITPID(cleanup, pids[1], &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child 1 returns %d", status);

		/* signal child_init_and_verify to verify its VM now */
		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

		SAFE_WAITPID(cleanup, pids[0], &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child 0 returns %d", status);
	}

	cleanup();
	tst_exit();
}

static void child_init_and_verify(void)
{
	unsigned char *foo;
	char buf[bufsz];
	long i, nr_err;

	foo = SAFE_MALLOC(tst_exit, bufsz);
	for (i = 0; i < bufsz; i++)
		foo[i] = DEFAULT_CHAR;
	tst_resm(TINFO, "child 0: memory allocated.");

	/* passing addr of string "foo" via pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	snprintf(buf, bufsz, "%p", foo);
	SAFE_WRITE(tst_exit, 1, pipe_fd[1], buf, strlen(buf) + 1);
	SAFE_CLOSE(tst_exit, pipe_fd[1]);

	/* wait until child_write() is done writing to our VM */
	TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);

	nr_err = 0;
	for (i = 0; i < bufsz; i++) {
		if (foo[i] != i % 256) {
#if DEBUG
			tst_resm(TFAIL, "child 0: expected %i, got %i for "
				 "byte seq %ld", i % 256, foo[i], i);
#endif
			nr_err++;
		}
	}
	if (nr_err)
		tst_brkm(TFAIL, tst_exit, "child 0: got %ld incorrect bytes.",
			 nr_err);
	else
		tst_resm(TPASS, "child 0: all bytes are expected.");
}

static void child_write(void)
{
	unsigned char *lp, *rp;
	char buf[bufsz];
	struct iovec local, remote;
	long i;

	/* get addr from pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[1]);
	SAFE_READ(tst_exit, 0, pipe_fd[0], buf, bufsz);
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	if (sscanf(buf, "%p", &rp) != 1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sscanf");

	lp = SAFE_MALLOC(tst_exit, bufsz + PADDING_SIZE * 2);

	for (i = 0; i < bufsz + PADDING_SIZE * 2; i++)
		lp[i] = DEFAULT_CHAR;
	for (i = 0; i < bufsz; i++)
		lp[i + PADDING_SIZE] = i % 256;

	local.iov_base = lp + PADDING_SIZE;
	local.iov_len = bufsz;
	remote.iov_base = rp;
	remote.iov_len = bufsz;

	tst_resm(TINFO, "child 2: write to the same memory location.");
	TEST(tst_syscall(__NR_process_vm_writev, pids[0], &local,
			 1UL, &remote, 1UL, 0UL));
	if (TEST_RETURN != bufsz)
		tst_brkm(TFAIL | TTERRNO, tst_exit, "process_vm_readv");
}

static void setup(void)
{
	tst_require_root();

	/* Just a sanity check of the existence of syscall */
	tst_syscall(__NR_process_vm_writev, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	bufsz =
	    sflag ? SAFE_STRTOL(NULL, sz_opt, 1, LONG_MAX - PADDING_SIZE * 2)
	    : 100000;

	tst_tmpdir();
	TST_CHECKPOINT_INIT(cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}

static void help(void)
{
	printf("    -s NUM  Set the size of total buffer size.\n");
}
