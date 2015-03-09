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

#include "test.h"
#include "safe_macros.h"
#include "lapi/tee.h"

#define TEST_FILE "testfile"

#define STR "abcdefghigklmnopqrstuvwxyz"
#define TEE_TEST_LEN 10

static int fd;
static int pipes[2];

static struct test_case_t {
	int *fdin;
	int *fdout;
	int exp_errno;
} test_cases[] = {
	{ &fd, &pipes[1], EINVAL },
	{ &pipes[0], &fd, EINVAL },
	{ &pipes[0], &pipes[1], EINVAL },
};

static void setup(void);
static void cleanup(void);
static void tee_verify(const struct test_case_t *);

char *TCID = "tee02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			tee_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_brkm(TCONF, cleanup, "This test can only run on kernels "
			"that are 2.6.17 or higher");
	}

	TEST_PAUSE;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDWR | O_CREAT);

	SAFE_PIPE(cleanup, pipes);
	SAFE_WRITE(cleanup, 1, pipes[1], STR, sizeof(STR) - 1);
}

static void tee_verify(const struct test_case_t *tc)
{
	TEST(tee(*(tc->fdin), *(tc->fdout), TEE_TEST_LEN, 0));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "tee() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "tee() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"tee() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	if (fd && close(fd) < 0)
		tst_resm(TWARN | TERRNO, "close fd failed");

	if (pipes[0] && close(pipes[0]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[0] failed");

	if (pipes[1] && close(pipes[1]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[1] failed");

	tst_rmdir();
}
