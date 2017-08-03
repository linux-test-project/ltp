/*
 * Copyright (c) International Business Machines  Corp., 2006
 *  Author Yi Yang <yyangcdl@cn.ibm.com>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * DESCRIPTION
 *	This test case will verify basic function of splice
 *	added by kernel 2.6.17 or up.
 *
 */

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>

#include "tst_test.h"
#include "lapi/splice.h"

#define TEST_BLOCK_SIZE 1024

#define TESTFILE1 "splice_testfile_1"
#define TESTFILE2 "splice_testfile_2"

static char buffer[TEST_BLOCK_SIZE];
static int fd_in, fd_out;

static void check_file(void)
{
	int i;
	char splicebuffer[TEST_BLOCK_SIZE];

	fd_out = SAFE_OPEN(TESTFILE2, O_RDONLY);
	SAFE_READ(1, fd_out, splicebuffer, TEST_BLOCK_SIZE);

	for (i = 0; i < TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != splicebuffer[i])
			break;
	}

	if (i < TEST_BLOCK_SIZE)
		tst_res(TFAIL, "Wrong data read from the buffer at %i", i);
	else
		tst_res(TPASS, "Written data has been read back correctly");

	SAFE_CLOSE(fd_out);
}

static void splice_test(void)
{
	int pipes[2];
	int ret;

	fd_in = SAFE_OPEN(TESTFILE1, O_RDONLY);
	fd_out = SAFE_OPEN(TESTFILE2, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	SAFE_PIPE(pipes);

	ret = splice(fd_in, NULL, pipes[1], NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(fd_in, pipe) failed");

	ret = splice(pipes[0], NULL, fd_out, NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(pipe, fd_out) failed");

	SAFE_CLOSE(fd_in);
	SAFE_CLOSE(fd_out);
	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);

	check_file();
}

static void setup(void)
{
	int i;

	if (tst_fs_type(".") == TST_NFS_MAGIC) {
		if  (tst_kvercmp(2, 6, 32) < 0)
			tst_brk(TCONF, "Cannot do splice on a file"
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
	.test_all = splice_test,
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
