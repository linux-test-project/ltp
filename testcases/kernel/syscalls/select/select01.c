// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2008-2025
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * http://www.sgi.com
 * Authors: Richard Logan, William Roske
 */

/*\
 * :manpage:`select(2)` with no I/O and small timeout to file descriptor of a
 *
 * - regular file
 * - system pipe
 * - named-pipe (FIFO)
 */

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include "select_var.h"

static fd_set readfds_reg, readfds_pipe, writefds_pipe, readfds_npipe, writefds_npipe;
static int fd_reg, fds_pipe[2], fd_npipe;

static struct tcases {
	int *nfds;
	fd_set *readfds;
	fd_set *writefds;
	int *readfd;
	int *writefd;
	char *desc;
} tests[] = {
	{&fd_reg, &readfds_reg, NULL, &fd_reg, NULL, "with regular file"},
	{&fds_pipe[1], &readfds_pipe, &writefds_pipe, &fds_pipe[0], &fds_pipe[1], "with system pipe"},
	{&fd_npipe, &readfds_npipe, &writefds_npipe, &fd_npipe, &fd_npipe, "with named pipe (FIFO)"},
};

static void run(unsigned int n)
{
	struct tcases *tc = &tests[n];
	struct timeval timeout;
	char buf;
	int exp_ret = 1;

	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	if (tc->writefd) {
		SAFE_WRITE(SAFE_WRITE_ANY, *tc->writefd, &buf, sizeof(buf));
		exp_ret++;
	}

	TEST(do_select(*tc->nfds + 1, tc->readfds, tc->writefds, 0, &timeout));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "select() %s failed", tc->desc);
		return;
	}

	if (!TST_RET) {
		tst_res(TFAIL, "select() %s timed out", tc->desc);
		return;
	}

	if (TST_RET != exp_ret) {
		tst_res(TFAIL, "select() %s returned %lu expected %d",
		        tc->desc, TST_RET, exp_ret);
		return;
	}

	tst_res(TPASS, "select() %s returned %i", tc->desc, exp_ret);

	if (FD_ISSET(*tc->readfd, tc->readfds))
		tst_res(TPASS, "readfds bit %i is set", *tc->readfd);
	else
		tst_res(TFAIL, "readfds bit %i is not set", *tc->readfd);

	if (!tc->writefd)
		return;

	if (FD_ISSET(*tc->writefd, tc->writefds))
		tst_res(TPASS, "writefds bit %i is set", *tc->writefd);
	else
		tst_res(TPASS, "writefds bit %i is not set", *tc->writefd);
}

static void setup(void)
{
	select_info();

	/* Regular file */
	fd_reg = SAFE_OPEN("tmpfile1", O_CREAT | O_RDWR, 0777);
	FD_ZERO(&readfds_reg);
	FD_SET(fd_reg, &readfds_reg);

	/* System pipe*/
	SAFE_PIPE(fds_pipe);
	FD_ZERO(&readfds_pipe);
	FD_ZERO(&writefds_pipe);
	FD_SET(fds_pipe[0], &readfds_pipe);
	FD_SET(fds_pipe[1], &writefds_pipe);

	/* Named pipe (FIFO) */
	SAFE_MKFIFO("tmpfile2", 0666);
	fd_npipe = SAFE_OPEN("tmpfile2", O_RDWR);
	FD_ZERO(&readfds_npipe);
	FD_ZERO(&writefds_npipe);
	FD_SET(fd_npipe, &readfds_npipe);
	FD_SET(fd_npipe, &writefds_npipe);
}

static void cleanup(void)
{
	SAFE_UNLINK("tmpfile2");
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = TEST_VARIANTS,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
