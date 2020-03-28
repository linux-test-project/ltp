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
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/select.h>

#include "lapi/fcntl.h"
#include "tst_test.h"

#define PAGE_NR 16
#define SECONDS 3
#define MICROSECONDS 0

static int fds[2];
static long page_size;

static void setup(void)
{
	/*
	 * Create the pipe with O_NONBLOCK.
	 */
	SAFE_PIPE2(fds, O_NONBLOCK);

	/*
	 * Get the system page size.
	 */
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);
}

static void test_pipe2(void)
{
	long flags;
	long pipe_size;

	char *buf;

	pid_t pid;
	int status;

	flags = SAFE_FCNTL(fds[0], F_GETFL);

	if (!(flags & O_NONBLOCK))
		tst_res(TFAIL, "O_NONBLOCK flag must be set.");

	pipe_size = SAFE_FCNTL(fds[0], F_GETPIPE_SZ);

	if (pipe_size != page_size * PAGE_NR)
		tst_res(TFAIL, "Default pipe page is 16 * 4096 = 65536B.");

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
		tst_res(TFAIL, "Pipe size (%ld) must be page size (%ld)",
			pipe_size, page_size);

	buf = alloca(page_size);

	SAFE_WRITE(1, fds[1], buf, page_size);

	/*
	 * This write should return -1 and errno set to either EAGAIN or
	 * EWOULDBLOCK because pipe is already full.
	 */
	if (write(fds[1], buf, page_size) != -1
		&& (errno != EAGAIN || errno != EWOULDBLOCK))
		tst_res(TFAIL | TERRNO, "write() succeeded and should not");

	SAFE_FCNTL(fds[1], F_SETFL, flags & ~O_NONBLOCK);

	flags = SAFE_FCNTL(fds[1], F_GETFL);

	if (flags & O_NONBLOCK)
		tst_res(TFAIL, "O_NONBLOCK flag must not be set.");

	pid = SAFE_FORK();

	/*
	 * Since writes are now blocking the child must wait forever on this
	 * write.
	 */
	if (!pid)
		SAFE_WRITE(1, fds[1], buf, page_size);

	if (TST_PROCESS_STATE_WAIT(pid, 'S', 1000))
		tst_res(TFAIL, "Child must be stopped.");
	else
		tst_res(TPASS, "Child is stopped.");

	SAFE_KILL(pid, SIGKILL);

	SAFE_WAIT(&status);
}

static void cleanup(void)
{
	for (int i = 0; i < 2; i++)
		if (fds[i] > 0)
			SAFE_CLOSE(fds[i]);
}

static struct tst_test test = {
	.test_all = test_pipe2,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};