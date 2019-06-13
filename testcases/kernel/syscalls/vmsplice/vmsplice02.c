// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
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
#include <limits.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"
#include "lapi/vmsplice.h"

#define TESTFILE "testfile"

#define TEST_BLOCK_SIZE 128

static char buffer[TEST_BLOCK_SIZE];
static int notvalidfd = -1;
static int filefd;
static int pipes[2];
static struct iovec ivc;

static struct tcase {
	int *fd;
	const struct iovec *iov;
	unsigned long nr_segs;
	int exp_errno;
} tcases[] = {
	{ &notvalidfd, &ivc, 1, EBADF },
	{ &filefd, &ivc, 1, EBADF },
	{ &pipes[1], &ivc, IOV_MAX + 1, EINVAL },
};

static void setup(void)
{
	if (tst_fs_type(".") == TST_NFS_MAGIC) {
		tst_brk(TCONF, "Cannot do splice() "
			"on a file located on an NFS filesystem");
	}

	filefd = SAFE_OPEN(TESTFILE, O_WRONLY | O_CREAT, 0644);

	SAFE_PIPE(pipes);

	ivc.iov_base = buffer;
	ivc.iov_len = TEST_BLOCK_SIZE;
}

static void vmsplice_verify(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(vmsplice(*(tc->fd), tc->iov, tc->nr_segs, 0));

	if (TST_RET != -1) {
		tst_res(TFAIL, "vmsplice() returned %ld, "
			"expected -1, errno:%d", TST_RET,
			tc->exp_errno);
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"vmsplice() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "vmsplice() failed as expected");
}

static void cleanup(void)
{
	if (filefd > 0)
		SAFE_CLOSE(filefd);

	if (pipes[0] > 0)
		SAFE_CLOSE(pipes[0]);

	if (pipes[1] > 0)
		SAFE_CLOSE(pipes[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = vmsplice_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
