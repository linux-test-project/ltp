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
 *   1) vmsplice() returns -1 and sets errno to EBADF if fd
 *      is not valid.
 *   2) vmsplice() returns -1 and sets errno to EBADF if fd
 *      doesn't refer to a pipe.
 *   3) vmsplice() returns -1 and sets errno to EINVAL if
 *      nr_segs is greater than IOV_MAX.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>

#include "test.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"
#include "tst_fs_type.h"
#include "lapi/fcntl.h"

#define TESTFILE "testfile"

#define TEST_BLOCK_SIZE 128

static char buffer[TEST_BLOCK_SIZE];
static int notvalidfd = -1;
static int filefd;
static int pipes[2];
static struct iovec ivc;

static struct test_case_t {
	int *fd;
	const struct iovec *iov;
	unsigned long nr_segs;
	int exp_errno;
} test_cases[] = {
	{ &notvalidfd, &ivc, 1, EBADF },
	{ &filefd, &ivc, 1, EBADF },
	{ &pipes[1], &ivc, IOV_MAX + 1, EINVAL },
};

static void setup(void);
static void cleanup(void);
static void vmsplice_verify(const struct test_case_t *);

char *TCID = "vmsplice02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			vmsplice_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_brkm(TCONF, cleanup, "This test can only run on "
			"kernels that are 2.6.17 or higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC) {
		tst_brkm(TCONF, cleanup, "Cannot do splice() "
			"on a file located on an NFS filesystem");
	}

	filefd = SAFE_OPEN(cleanup, TESTFILE,
				O_WRONLY | O_CREAT, 0644);

	SAFE_PIPE(cleanup, pipes);

	ivc.iov_base = buffer;
	ivc.iov_len = TEST_BLOCK_SIZE;
}

static void vmsplice_verify(const struct test_case_t *tc)
{
	TEST(vmsplice(*(tc->fd), tc->iov, tc->nr_segs, 0));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "vmsplice() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "vmsplice() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"vmsplice() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	if (filefd && close(filefd) < 0)
		tst_resm(TWARN | TERRNO, "close filefd failed");

	if (pipes[0] && close(pipes[0]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[0] failed");

	if (pipes[1] && close(pipes[1]) < 0)
		tst_resm(TWARN | TERRNO, "close pipes[1] failed");

	tst_rmdir();
}
