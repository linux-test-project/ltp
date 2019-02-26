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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "process_vm_readv03";
int TST_TOTAL = 1;

#define NUM_LOCAL_VECS 4

static int nflag, sflag;
static char *nr_opt, *sz_opt;
static option_t options[] = {
	{"n:", &nflag, &nr_opt},
	{"s:", &sflag, &sz_opt},
	{NULL, NULL, NULL}
};

static int nr_iovecs;
static long bufsz;
static int pipe_fd[2];
static pid_t pids[2];

static void gen_random_arr(int *arr, int arr_sz);
static void child_alloc(int *bufsz_arr);
static void child_invoke(int *bufsz_arr);
static long *fetch_remote_addrs(void);
static void setup(void);
static void cleanup(void);
static void help(void);

int main(int argc, char **argv)
{
	int lc, status;
	int *bufsz_arr;

	tst_parse_opts(argc, argv, options, &help);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		SAFE_PIPE(cleanup, pipe_fd);

		bufsz_arr = SAFE_MALLOC(cleanup, nr_iovecs * sizeof(int));
		gen_random_arr(bufsz_arr, nr_iovecs);

		/* the start of child_alloc and child_invoke is already
		 * synchronized via pipe */
		pids[0] = fork();
		switch (pids[0]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #0");
		case 0:
			child_alloc(bufsz_arr);
			exit(0);
		}

		pids[1] = fork();
		switch (pids[1]) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork #1");
		case 0:
			child_invoke(bufsz_arr);
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

		free(bufsz_arr);
	}

	cleanup();
	tst_exit();
}

static void gen_random_arr(int *arr, int arr_sz)
{
	long bufsz_left, bufsz_single;
	int i;

	bufsz_left = bufsz;
	for (i = 0; i < arr_sz - 1; i++) {
		bufsz_single = rand() % (bufsz_left / 2) + 1;
		arr[i] = bufsz_single;
		bufsz_left -= bufsz_single;
	}
	arr[arr_sz - 1] = bufsz_left;
}

static void child_alloc(int *bufsz_arr)
{
	char **foo;
	int i, j;
	char buf[BUFSIZ];
	long count;

	foo = SAFE_MALLOC(tst_exit, nr_iovecs * sizeof(char *));

	count = 0;
	for (i = 0; i < nr_iovecs; i++) {
		foo[i] = SAFE_MALLOC(tst_exit, bufsz_arr[i]);
		for (j = 0; j < bufsz_arr[i]; j++) {
			foo[i][j] = count % 256;
			count++;
		}
	}
	tst_resm(TINFO, "child 0: %d iovecs allocated and initialized.",
		 nr_iovecs);

	/* passing addr via pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	snprintf(buf, BUFSIZ, "%p", (void *)foo);
	SAFE_WRITE(tst_exit, 1, pipe_fd[1], buf, strlen(buf) + 1);
	SAFE_CLOSE(tst_exit, pipe_fd[1]);

	/* wait until child_invoke is done reading from our VM */
	TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);
}

static long *fetch_remote_addrs(void)
{
	long *foo, *bar;
	char buf[BUFSIZ];
	long len;
	struct iovec local, remote;

	/* get addr from pipe */
	SAFE_CLOSE(tst_exit, pipe_fd[1]);
	SAFE_READ(tst_exit, 0, pipe_fd[0], buf, BUFSIZ);
	SAFE_CLOSE(tst_exit, pipe_fd[0]);
	if (sscanf(buf, "%p", &foo) != 1)
		tst_brkm(TBROK | TERRNO, tst_exit, "sscanf");

	len = nr_iovecs * sizeof(long);
	bar = SAFE_MALLOC(tst_exit, len);
	local.iov_base = bar;
	local.iov_len = len;
	remote.iov_base = foo;
	remote.iov_len = len;

	TEST(ltp_syscall(__NR_process_vm_readv, pids[0], &local,
			 1UL, &remote, 1UL, 0UL));
	if (TEST_RETURN != len)
		tst_brkm(TFAIL | TERRNO, tst_exit, "process_vm_readv");

	return local.iov_base;
}

static void child_invoke(int *bufsz_arr)
{
	int i, j, count, nr_error;
	unsigned char expect, actual;
	long *addrs;
	struct iovec local[NUM_LOCAL_VECS], *remote;
	int rcv_arr[NUM_LOCAL_VECS];

	addrs = fetch_remote_addrs();

	remote = SAFE_MALLOC(tst_exit, nr_iovecs * sizeof(struct iovec));
	for (i = 0; i < nr_iovecs; i++) {
		remote[i].iov_base = (void *)addrs[i];
		remote[i].iov_len = bufsz_arr[i];
	}
	tst_resm(TINFO, "child 1: %d remote iovecs received.", nr_iovecs);

	gen_random_arr(rcv_arr, NUM_LOCAL_VECS);
	for (i = 0; i < NUM_LOCAL_VECS; i++) {
		local[i].iov_base = SAFE_MALLOC(tst_exit, rcv_arr[i]);
		local[i].iov_len = rcv_arr[i];
	}
	tst_resm(TINFO, "child 1: %d local iovecs initialized.",
		 NUM_LOCAL_VECS);

	TEST(ltp_syscall(__NR_process_vm_readv, pids[0], local,
			    (unsigned long)NUM_LOCAL_VECS, remote,
			    (unsigned long)nr_iovecs, 0UL));
	if (TEST_RETURN != bufsz)
		tst_brkm(TBROK | TERRNO, tst_exit, "process_vm_readv");

	/* verify every byte */
	count = 0;
	nr_error = 0;
	for (i = 0; i < NUM_LOCAL_VECS; i++) {
		for (j = 0; j < local[i].iov_len; j++) {
			expect = count % 256;
			actual = ((unsigned char *)local[i].iov_base)[j];
			if (expect != actual) {
#if DEBUG
				tst_resm(TFAIL, "child 1: expected %i, got %i "
					 "for byte seq %d",
					 expect, actual, count);
#endif
				nr_error++;
			}
			count++;
		}
	}
	if (nr_error)
		tst_brkm(TFAIL, tst_exit, "child 1: %d incorrect bytes "
			 "received.", nr_error);
	else
		tst_resm(TPASS, "child 1: all bytes are correctly received.");
}

static void setup(void)
{
	tst_require_root();

	/* Just a sanity check of the existence of syscall */
	ltp_syscall(__NR_process_vm_readv, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	nr_iovecs = nflag ? SAFE_STRTOL(NULL, nr_opt, 1, IOV_MAX) : 10;
	bufsz = sflag ? SAFE_STRTOL(NULL, sz_opt, NUM_LOCAL_VECS, LONG_MAX)
	    : 100000;

	tst_tmpdir();
	TST_CHECKPOINT_INIT(cleanup);
	srand(time(NULL));

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}

static void help(void)
{
	printf("    -n NUM  Set the number of iovecs to be allocated.\n");
	printf("    -s NUM  Set the size of total buffer size.\n");
}
