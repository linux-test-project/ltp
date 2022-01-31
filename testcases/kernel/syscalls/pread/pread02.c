// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Tests basic error handling of the pread syscall.
 *
 * - ESPIPE when attempted to read from an unnamed pipe
 * - EINVAL if the specified offset position was invalid
 * - EISDIR when fd refers to a directory
 */

#include <fcntl.h>
#include <stdlib.h>
#include "tst_test.h"

#define PREAD_TEMPFILE  "pread_file"
#define PREAD_TEMPDIR	"pread_dir"
#define K1              1024

static int pipe_fd[2], fd, dir_fd;

struct test_case_t {
	int *fd;
	size_t nb;
	off_t offst;
	char *desc;
	int exp_errno;
} tcases[] = {
	{&pipe_fd[0], K1, 0, "file descriptor is a PIPE or FIFO", ESPIPE},
	{&fd, K1, -1, "specified offset is negative", EINVAL},
	{&dir_fd, K1, 0, "file descriptor is a directory", EISDIR}
};

static void verify_pread(unsigned int n)
{
	struct test_case_t *tc = &tcases[n];
	char buf[K1];

	TST_EXP_FAIL2(pread(*tc->fd, &buf, tc->nb, tc->offst), tc->exp_errno,
		"pread(%d, %zu, %ld) %s", *tc->fd, tc->nb, tc->offst, tc->desc);
}

static void setup(void)
{
	SAFE_PIPE(pipe_fd);
	SAFE_WRITE(1, pipe_fd[1], "x", 1);

	fd = SAFE_OPEN(PREAD_TEMPFILE, O_RDWR | O_CREAT, 0666);

	SAFE_MKDIR(PREAD_TEMPDIR, 0777);
	dir_fd = SAFE_OPEN(PREAD_TEMPDIR, O_RDONLY);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (pipe_fd[i] > 0)
			SAFE_CLOSE(pipe_fd[i]);
	}

	if (fd > 0)
		SAFE_CLOSE(fd);
	if (dir_fd > 0)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_pread,
};
