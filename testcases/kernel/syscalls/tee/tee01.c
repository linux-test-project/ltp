/****************************************************************************
 *
 * Copyright (c) International Business Machines  Corp., 2006
 *  Author Yi Yang <yyangcdl@cn.ibm.com>
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 *
 * DESCRIPTION
 *	This test case will verify basic function of tee(2)
 *	added by kernel 2.6.17 or up.
 *
 ***************************************************************************/

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>

#include "test.h"
#include "linux_syscall_numbers.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "lapi/tee.h"
#include "lapi/splice.h"

static void tee_test(void);
static void setup(void);
static void cleanup(void);

#define TEST_BLOCK_SIZE 1024

#define TESTFILE1 "tee_test_file_1"
#define TESTFILE2 "tee_test_file_2"

static int fd_in, fd_out;
static char buffer[TEST_BLOCK_SIZE];

char *TCID = "tee01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		tee_test();

	cleanup();
	tst_exit();
}

static void check_file(void)
{
	int i;
	char teebuffer[TEST_BLOCK_SIZE];

	fd_out = SAFE_OPEN(cleanup, TESTFILE2, O_RDONLY);
	SAFE_READ(cleanup, 1, fd_out, teebuffer, TEST_BLOCK_SIZE);

	for (i = 0; i < TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != teebuffer[i])
			break;
	}

	if (i < TEST_BLOCK_SIZE)
		tst_resm(TFAIL, "Wrong data read from the buffer at %i", i);
	else
		tst_resm(TPASS, "Written data has been read back correctly");

	close(fd_out);
	fd_out = 0;
}

static void tee_test(void)
{
	int pipe1[2];
	int pipe2[2];
	int ret = 0;

	fd_in = SAFE_OPEN(cleanup, TESTFILE1, O_RDONLY);
	fd_out = SAFE_OPEN(cleanup, TESTFILE2, O_WRONLY | O_CREAT | O_TRUNC, 0777);

	SAFE_PIPE(cleanup, pipe1);
	SAFE_PIPE(cleanup, pipe2);

	ret = splice(fd_in, NULL, pipe1[1], NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "splice(fd_in, pipe1) failed");

	ret = tee(pipe1[0], pipe2[1], TEST_BLOCK_SIZE, SPLICE_F_NONBLOCK);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "tee() failed");

	ret = splice(pipe2[0], NULL, fd_out, NULL, TEST_BLOCK_SIZE, 0);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "splice(pipe2, fd_out) failed");

	close(pipe2[0]);
	close(pipe2[1]);
	close(pipe1[0]);
	close(pipe1[1]);
	close(fd_out);
	close(fd_in);

	fd_out = 0;
	fd_in = 0;

	check_file();
}

static void setup(void)
{
	int i;

	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_brkm(TCONF, cleanup, "This test can only run on kernels "
			"that are 2.6.17 or higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC) {
		if ((tst_kvercmp(2, 6, 32)) < 0)
			tst_brkm(TCONF, cleanup, "Cannot do tee on a file"
				" on NFS filesystem before 2.6.32");
	}

	for (i = 0; i < TEST_BLOCK_SIZE; i++)
		buffer[i] = i & 0xff;

	fd_in = SAFE_OPEN(cleanup, TESTFILE1, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	SAFE_WRITE(cleanup, 1, fd_in, buffer, TEST_BLOCK_SIZE);
	SAFE_CLOSE(cleanup, fd_in);
	fd_in = 0;
}

static void cleanup(void)
{
	if (fd_in > 0 && close(fd_in))
		tst_resm(TWARN, "Failed to close fd_in");

	if (fd_out > 0 && close(fd_out))
		tst_resm(TWARN, "Failed to close fd_out");

	tst_rmdir();
}
