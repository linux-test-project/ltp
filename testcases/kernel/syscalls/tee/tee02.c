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
 *   1) tee() returns -1 and sets errno to EINVAL if fd_in does
 *      not refer to a pipe.
 *   2) tee() returns -1 and sets errno to EINVAL if fd_out does
 *      not refer to a pipe.
 *   3) tee() returns -1 and sets errno to EINVAL if fd_in and
 *      fd_out refer to the same pipe.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/tee.h"

#define TEST_FILE "testfile"

#define STR "abcdefghigklmnopqrstuvwxyz"
#define TEE_TEST_LEN 10

static int fd;
static int pipes[2];

static struct tcase {
	int *fdin;
	int *fdout;
	int exp_errno;
} tcases[] = {
	{ &fd, &pipes[1], EINVAL },
	{ &pipes[0], &fd, EINVAL },
	{ &pipes[0], &pipes[1], EINVAL },
};

static void setup(void)
{
	fd = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT);
	SAFE_PIPE(pipes);
	SAFE_WRITE(1, pipes[1], STR, sizeof(STR) - 1);
}

static void tee_verify(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(tee(*(tc->fdin), *(tc->fdout), TEE_TEST_LEN, 0));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "tee() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"tee() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "tee() failed as expected");
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (pipes[0] > 0)
		SAFE_CLOSE(pipes[0]);

	if (pipes[1] > 0)
		SAFE_CLOSE(pipes[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = tee_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
