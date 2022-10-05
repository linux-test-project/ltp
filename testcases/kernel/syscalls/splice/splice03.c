// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
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

#include "tst_test.h"
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

static struct tcase {
	int *fdin;
	loff_t *offin;
	int *fdout;
	loff_t *offout;
	int exp_errno;
} tcases[] = {
	{ &badfd, NULL, &pipes[1], NULL, EBADF },
	{ &pipes[0], NULL, &badfd, NULL, EBADF },
	{ &wrfd, NULL, &pipes[1], NULL, EBADF },
	{ &pipes[0], NULL, &appendfd, NULL, EINVAL },
	{ &rdfd, NULL, &wrfd, NULL, EINVAL },
	{ &pipes[0], &offset, &wrfd, NULL, ESPIPE },
	{ &rdfd, NULL, &pipes[1], &offset, ESPIPE },
};

static void setup(void)
{
	SAFE_FILE_PRINTF(TEST_FILE, STR);
	rdfd = SAFE_OPEN(TEST_FILE, O_RDONLY);

	wrfd = SAFE_OPEN(TEST_FILE2, O_WRONLY | O_CREAT, 0644);

	appendfd = SAFE_OPEN(TEST_FILE3, O_RDWR | O_CREAT | O_APPEND, 0644);

	SAFE_PIPE(pipes);

	SAFE_WRITE(SAFE_WRITE_ALL, pipes[1], STR, sizeof(STR) - 1);
}

static void splice_verify(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(splice(*(tc->fdin), tc->offin, *(tc->fdout),
		tc->offout, SPLICE_TEST_LEN, 0));

	if (TST_RET != -1) {
		tst_res(TFAIL, "splice() returned %ld expected %s",
			TST_RET, tst_strerrno(tc->exp_errno));
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"splice() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "splice() failed as expected");
}

static void cleanup(void)
{
	if (rdfd > 0)
		SAFE_CLOSE(rdfd);

	if (wrfd > 0)
		SAFE_CLOSE(wrfd);

	if (appendfd > 0)
		SAFE_CLOSE(appendfd);

	if (pipes[0] > 0)
		SAFE_CLOSE(pipes[0]);

	if (pipes[1] > 0)
		SAFE_CLOSE(pipes[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = splice_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
