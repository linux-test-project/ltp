// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    07/2001 Ported by Wayne Boyer
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies inheritance of file descriptors from parent to child
 * process. We open a file from parent, then we check if file offset changes
 * accordingly with file descriptor usage.
 *
 * [Algorithm]
 *
 * Test steps are the following:
 * - create a file made in three parts -> | aa..a | bb..b | cc..c |
 * - from parent, open the file
 * - from child, move file offset after the first part
 * - from parent, read second part and check if it's | bb..b |
 * - from child, read third part and check if it's | cc..c |
 *
 * Test passes if we were able to read the correct file parts from parent and
 * child.
 */

#include <stdlib.h>
#include "tst_test.h"

#define FILENAME "file.txt"
#define DATASIZE 1024

static int fd = -1;

static void run(void)
{
	int status;
	char buff[DATASIZE];
	char data[DATASIZE];

	fd = SAFE_OPEN(FILENAME, 0);

	if (!SAFE_FORK()) {
		SAFE_LSEEK(fd, DATASIZE, SEEK_SET);
		exit(0);
	}

	SAFE_WAIT(&status);

	memset(buff, 'b', DATASIZE);
	SAFE_READ(1, fd, data, DATASIZE);

	TST_EXP_EXPR(strncmp(buff, data, DATASIZE) == 0,
		"read first part of data from parent process");

	if (!SAFE_FORK()) {
		memset(buff, 'c', DATASIZE);
		SAFE_READ(1, fd, data, DATASIZE);

		TST_EXP_EXPR(strncmp(buff, data, DATASIZE) == 0,
			"read second part of data from child process");

		exit(0);
	}

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	char buff[DATASIZE];

	fd = SAFE_CREAT(FILENAME, 0600);

	memset(buff, 'a', DATASIZE);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buff, DATASIZE);

	memset(buff, 'b', DATASIZE);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buff, DATASIZE);

	memset(buff, 'c', DATASIZE);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buff, DATASIZE);

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
