/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Description:
 *   Verify that,
 *   1) splice() returns -1 and sets errno to EBADF if the file
 *      descriptor fd_in is not valid.
 *   2) splice() returns -1 and sets errno to EBADF if the file
 *      descriptor fd_out is not valid.
 *   3) splice() returns -1 and sets errno to EBADF if the file
 *      descriptor fd_in does not have proper read-write mode.
 *   4) splice() returns -1 and sets errno to EINVAL if target
 *      file is opened in append mode.
 *   5) splice() returns -1 and sets errno to EINVAL if neither
 *      of the descriptors refer to a pipe.
 *   6) splice() returns -1 and sets errno to ESPIPE if off_in is
 *      not NULL when the file descriptor fd_in refers to a pipe.
 *   7) splice() returns -1 and sets errno to ESPIPE if off_out is
 *      not NULL when the file descriptor fd_out refers to a pipe.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/splice.h"

#define TEST_FILE "testfile"
#define TEST_FILE2 "testfile2"
#define TEST_FILE3 "testfile3"

#define STR "abcdefghigklmnopqrstuvwxyz"
#define SPLICE_TEST_LEN 10

static int badfd = -1;
static int rdfd;
static int wrfd;
static int appendfd;
static int pipes[2];
static loff_t offset;

static struct test_case_t {
	int *fdin;
	loff_t *offin;
	int *fdout;
	loff_t *offout;
	int exp_errno;
} test_cases[] = {
	{ &badfd, NULL, &pipes[1], NULL, EBADF },
	{ &pipes[0], NULL, &badfd, NULL, EBADF },
	{ &wrfd, NULL, &pipes[1], NULL, EBADF },
	{ &pipes[0], NULL, &appendfd, NULL, EINVAL },
	{ &rdfd, NULL, &wrfd, NULL, EINVAL },
	{ &pipes[0], &offset, &wrfd, NULL, ESPIPE },
	{ &rdfd, NULL, &pipes[1], &offset, ESPIPE },
};

static void setup(void);
static void cleanup(void);
static void splice_verify(const struct test_case_t *);

char *TCID = "splice03";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			splice_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_brkm(TCONF, cleanup, "This test can only run on kernels "
			"that are 2.6.17 or higher");
	}

	TEST_PAUSE;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_FILE_PRINTF(cleanup, TEST_FILE, STR);
	rdfd = SAFE_OPEN(cleanup, TEST_FILE, O_RDONLY);

	wrfd = SAFE_OPEN(cleanup, TEST_FILE2,
		O_WRONLY | O_CREAT, 0644);

	appendfd = SAFE_OPEN(cleanup, TEST_FILE3,
		O_RDWR | O_CREAT | O_APPEND, 0644);

	SAFE_PIPE(cleanup, pipes);

	SAFE_WRITE(cleanup, 1, pipes[1], STR, sizeof(STR) - 1);
}

static void splice_verify(const struct test_case_t *tc)
{
	TEST(splice(*(tc->fdin), tc->offin, *(tc->fdout),
		tc->offout, SPLICE_TEST_LEN, 0));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "splice() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "splice() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"splice() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}
}

void cleanup(void)
{
	if (rdfd && close(rdfd) < 0)
		tst_resm(TWARN | TERRNO, "close rdfd failed");

	if (wrfd && close(wrfd) < 0)
		tst_resm(TWARN | TERRNO, "close wrfd failed");

	if (appendfd && close(appendfd) < 0)
		tst_resm(TWARN | TERRNO, "close appendfd failed");

	if (pipes[0] && close(pipes[0]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[0] failed");

	if (pipes[1] && close(pipes[1]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[1] failed");

	tst_rmdir();
}
