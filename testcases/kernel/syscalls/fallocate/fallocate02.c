// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Author: Sharyathi Nagesh <sharyathi@in.ibm.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Verify that :manpage:`fallocate(2)` fails with the expected error codes:
 *
 * - EBADF when the file descriptor is opened read-only.
 * - EINVAL when the offset or length is negative, or the length is zero.
 * - EFBIG when the requested range exceeds the maximum file size (only
 *   tested on 64-bit offset ABIs).
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fallocate.h"
#include "lapi/abisize.h"

#define BLOCKS_WRITTEN	12
#define OFFSET		12
#define FNAMER		"test_file1"
#define FNAMEW		"test_file2"
#define BLOCK_SIZE	1024
#define MAX_FILESIZE	(LLONG_MAX / BLOCK_SIZE)

static int fdr = -1;
static int fdw = -1;

static struct tcase {
	int *fd;
	loff_t offset;
	loff_t len;
	int exp_errno;
} tcases[] = {
	{&fdr, 0, 1, EBADF},
	{&fdw, -1, 1, EINVAL},
	{&fdw, 1, -1, EINVAL},
	{&fdw, BLOCKS_WRITTEN, 0, EINVAL},
	{&fdw, BLOCKS_WRITTEN, -1, EINVAL},
	{&fdw, -(BLOCKS_WRITTEN + OFFSET), 1, EINVAL},
#if defined(TST_ABI64) || _FILE_OFFSET_BITS == 64
	{&fdw, MAX_FILESIZE, 1, EFBIG},
	{&fdw, 1, MAX_FILESIZE, EFBIG},
#endif
};

static void setup(void)
{
	char buf[BLOCK_SIZE];
	int i;

	TEST(fallocate(-1, 0, 0, 0));
	if (TST_ERR == EOPNOTSUPP || TST_ERR == ENOSYS)
		tst_brk(TCONF, "fallocate() not supported");

	fdr = SAFE_OPEN(FNAMER, O_RDONLY | O_CREAT, 0400);
	fdw = SAFE_OPEN(FNAMEW, O_RDWR | O_CREAT, 0700);

	memset(buf, 'A', BLOCK_SIZE);
	for (i = 0; i < BLOCKS_WRITTEN; i++)
		SAFE_WRITE(SAFE_WRITE_ALL, fdw, buf, BLOCK_SIZE);
}

static void cleanup(void)
{
	if (fdw != -1)
		SAFE_CLOSE(fdw);
	if (fdr != -1)
		SAFE_CLOSE(fdr);
}

static void verify_fallocate(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(fallocate(*tc->fd, FALLOC_FL_KEEP_SIZE,
			       tc->offset * BLOCK_SIZE,
			       tc->len * BLOCK_SIZE), tc->exp_errno,
		     "fallocate(%d, %lld, %lld)", *tc->fd,
		     (long long)(tc->offset * BLOCK_SIZE),
		     (long long)(tc->len * BLOCK_SIZE));
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fallocate,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
};
