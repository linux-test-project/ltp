// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Francis Laniel. All rights reserved.
 * Author: Francis Laniel <laniel_francis@privacyrequired.com>
 *
 * Test Description:
 * This Program tests getting and setting the pipe size.
 * It also tests what happen when you write to a full pipe depending on whether
 * O_NONBLOCK is set or not.
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <features.h>
#include <unistd.h>
#include <stdio.h>
#include "lapi/fcntl.h"
#include "tst_test.h"

static int fds[2];
static int flags;

static void test_pipe2(void)
{
	int ret;
	pid_t pid;

	/*
	 * This ensures parent process is still in non-block mode when
	 * using -i parameter. Subquent writes hould return -1 and errno
	 * set to either EAGAIN or EWOULDBLOCK because pipe is already full.
	 */
	SAFE_FCNTL(fds[1], F_SETFL, flags | O_NONBLOCK);
	ret = write(fds[1], "x", 1);
	if (ret == -1) {
		if (errno == EAGAIN)
			tst_res(TPASS | TERRNO, "write failed as expected");
		else
			tst_brk(TFAIL | TERRNO, "write failed expected EAGAIN but got");
	} else {
		tst_res(TFAIL, "write() succeeded unexpectedly");
	}

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_FCNTL(fds[1], F_SETFL, flags & ~O_NONBLOCK);
		SAFE_WRITE(1, fds[1], "x", 1);
	}

	if (TST_PROCESS_STATE_WAIT(pid, 'S', 1000) < 0)
		tst_res(TFAIL, "Child process is not blocked");
	else
		tst_res(TPASS, "Child process is blocked");

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAIT(NULL);
}

static void setup(void)
{
	int page_size, pipe_size;
	char *write_buffer;

	SAFE_PIPE2(fds, O_NONBLOCK);
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);

	flags = SAFE_FCNTL(fds[1], F_GETFL);
	if (!(flags & O_NONBLOCK))
		tst_brk(TCONF, "O_NONBLOCK flag must be set");
	/*
	 * A pipe has two file descriptors.
	 * But in the kernel these two file descriptors point to the same pipe.
	 * So setting size from first file handle set size for the pipe.
	 */
	SAFE_FCNTL(fds[0], F_SETPIPE_SZ, 0);

	/*
	 * So getting size from the second file descriptor return the size of
	 * the pipe which was changed before with first file descriptor.
	 */
	pipe_size = SAFE_FCNTL(fds[1], F_GETPIPE_SZ);
	if (pipe_size != page_size)
		tst_res(TFAIL, "Pipe size (%d) must be page size (%d)",
				pipe_size, page_size);

	write_buffer = SAFE_MALLOC(pipe_size);
	memset(write_buffer, 'x', pipe_size);
	SAFE_WRITE(1, fds[1], write_buffer, pipe_size);
	free(write_buffer);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.min_kver = "2.6.35",
	.test_all = test_pipe2,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
