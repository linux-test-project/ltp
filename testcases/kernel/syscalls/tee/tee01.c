// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2006
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Author: Yi Yang <yyangcdl@cn.ibm.com>
 */

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/tee.h"
#include "lapi/splice.h"

#define TEST_BLOCK_SIZE 1024

#define TESTFILE1 "tee_test_file_1"
#define TESTFILE2 "tee_test_file_2"

static int fd_in, fd_out;
static char buffer[TEST_BLOCK_SIZE];

static void check_file(void)
{
	int i;
	char teebuffer[TEST_BLOCK_SIZE];

	fd_out = SAFE_OPEN(TESTFILE2, O_RDONLY);
	SAFE_READ(1, fd_out, teebuffer, TEST_BLOCK_SIZE);

	for (i = 0; i < TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != teebuffer[i])
			break;
	}

	if (i < TEST_BLOCK_SIZE)
		tst_res(TFAIL, "Wrong data read from the buffer at %i", i);
	else
		tst_res(TPASS, "Written data has been read back correctly");

	SAFE_CLOSE(fd_out);
}

static void tee_test(void)
{
	int pipe1[2];
	int pipe2[2];
	int ret = 0;

	fd_in = SAFE_OPEN(TESTFILE1, O_RDONLY);
	fd_out = SAFE_OPEN(TESTFILE2, O_WRONLY | O_CREAT | O_TRUNC, 0777);

	SAFE_PIPE(pipe1);
	SAFE_PIPE(pipe2);

	ret = splice(fd_in, NULL, pipe1[1], NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(fd_in, pipe1) failed");

	ret = tee(pipe1[0], pipe2[1], TEST_BLOCK_SIZE, SPLICE_F_NONBLOCK);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "tee() failed");

	ret = splice(pipe2[0], NULL, fd_out, NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(pipe2, fd_out) failed");

	SAFE_CLOSE(pipe2[0]);
	SAFE_CLOSE(pipe2[1]);
	SAFE_CLOSE(pipe1[0]);
	SAFE_CLOSE(pipe1[1]);
	SAFE_CLOSE(fd_out);
	SAFE_CLOSE(fd_in);

	check_file();
}

static void setup(void)
{
	int i;

	if (tst_fs_type(".") == TST_NFS_MAGIC) {
		if ((tst_kvercmp(2, 6, 32)) < 0)
			tst_brk(TCONF, "Cannot do tee on a file"
				" on NFS filesystem before 2.6.32");
	}

	for (i = 0; i < TEST_BLOCK_SIZE; i++)
		buffer[i] = i & 0xff;

	fd_in = SAFE_OPEN(TESTFILE1, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	SAFE_WRITE(1, fd_in, buffer, TEST_BLOCK_SIZE);
	SAFE_CLOSE(fd_in);
}

static void cleanup(void)
{
	if (fd_in > 0)
		SAFE_CLOSE(fd_in);

	if (fd_out > 0)
		SAFE_CLOSE(fd_out);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = tee_test,
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
