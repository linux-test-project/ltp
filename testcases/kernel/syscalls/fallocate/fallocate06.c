// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Tests misaligned fallocate()
 *
 * Test scenario:
 *
 * 1. write() several blocks worth of data
 * 2. fallocate() some more space (not aligned to FS blocks)
 * 3. try to write() into the allocated space
 * 4. deallocate misaligned part of file range written in step 1
 * 5. read() the deallocated range and check that it was zeroed
 *
 * Subtests:
 *
 * - fill filesystem between step 2 and 3
 * - disable copy-on-write on test file
 * - combinations of above subtests
 */

/*
 * This is also regression test for:
 * e093c4be760e ("xfs: Fix tail rounding in xfs_alloc_file_space()")
 * 6d4572a9d71d ("Allow btrfs_truncate_block() to fallback to nocow for data
 *              space reservation")
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "tst_test.h"
#include "lapi/fallocate.h"

#define MNTPOINT "mntpoint"
#define TEMPFILE MNTPOINT "/test_file"
#define WRITE_BLOCKS 8
#define FALLOCATE_BLOCKS 2
#define DEALLOCATE_BLOCKS 3
#define TESTED_FLAGS "fallocate(FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE)"

const struct test_case {
	int no_cow, fill_fs;
} testcase_list[] = {
	{1, 0},
	{1, 1},
	{0, 0},
	{0, 1}
};

static int cow_support;
static char *wbuf, *rbuf;
static blksize_t blocksize;
static long wbuf_size, rbuf_size, block_offset;

static int toggle_cow(int fd, int enable)
{
	int ret, attr;

	ret = ioctl(fd, FS_IOC_GETFLAGS, &attr);

	if (ret)
		return ret;

	if (enable)
		attr &= ~FS_NOCOW_FL;
	else
		attr |= FS_NOCOW_FL;

	return ioctl(fd, FS_IOC_SETFLAGS, &attr);
}

static void setup(void)
{
	unsigned char ch;
	long i;
	int fd;
	struct stat statbuf;

	fd = SAFE_OPEN(TEMPFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	/*
	 * Set FS_NOCOW_FL flag on the temp file. Non-CoW filesystems will
	 * return error.
	 */
	TEST(toggle_cow(fd, 0));
	SAFE_FSTAT(fd, &statbuf);
	blocksize = statbuf.st_blksize;
	block_offset = MIN(blocksize / 2, 512);
	wbuf_size = MAX(WRITE_BLOCKS, FALLOCATE_BLOCKS) * blocksize;
	rbuf_size = (DEALLOCATE_BLOCKS + 1) * blocksize;
	SAFE_CLOSE(fd);
	SAFE_UNLINK(TEMPFILE);

	if (blocksize < 2)
		tst_brk(TCONF, "Block size %ld too small for test", blocksize);

	if (!TST_RET) {
		cow_support = 1;
	} else {
		switch (TST_ERR) {
		case ENOTSUP:
		case ENOTTY:
		case EINVAL:
		case ENOSYS:
			cow_support = 0;
			break;

		default:
			tst_brk(TBROK|TTERRNO,
				"Error checking copy-on-write support");
			break;
		}
	}

	tst_res(TINFO, "Copy-on-write is%s supported",
		cow_support ? "" : " not");
	wbuf = SAFE_MALLOC(wbuf_size);
	rbuf = SAFE_MALLOC(rbuf_size);

	/* Fill the buffer with known values */
	for (i = 0, ch = 1; i < wbuf_size; i++, ch++)
		wbuf[i] = ch;
}

static int check_result(const struct test_case *tc, const char *func, long exp)
{
	if (tc->fill_fs && !tc->no_cow && TST_RET < 0) {
		if (TST_RET != -1) {
			tst_res(TFAIL, "%s returned unexpected value %ld",
				func, TST_RET);
			return 0;
		}

		if (TST_ERR != ENOSPC) {
			tst_res(TFAIL | TTERRNO, "%s should fail with ENOSPC",
				func);
			return 0;
		}

		tst_res(TPASS | TTERRNO, "%s on full FS with CoW", func);
		return 1;
	}

	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "%s failed unexpectedly", func);
		return 0;
	}

	if (TST_RET != exp) {
		tst_res(TFAIL,
			"Unexpected return value from %s: %ld (expected %ld)",
			func, TST_RET, exp);
		return 0;
	}

	tst_res(TPASS, "%s successful", func);
	return 1;
}

static void run(unsigned int n)
{
	int fd, i, err;
	long offset, size;
	const struct test_case *tc = testcase_list + n;

	tst_res(TINFO, "Case %u. Fill FS: %s; Use copy on write: %s", n+1,
		tc->fill_fs ? "yes" : "no", tc->no_cow ? "no" : "yes");
	fd = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if (cow_support)
		toggle_cow(fd, !tc->no_cow);
	else if (!tc->no_cow)
		tst_brk(TCONF, "File system does not support copy-on-write");

	/* Prepare test data for deallocation test */
	size = WRITE_BLOCKS * blocksize;
	SAFE_WRITE(1, fd, wbuf, size);

	/* Allocation test */
	offset = size + block_offset;
	size = FALLOCATE_BLOCKS * blocksize;
	TEST(fallocate(fd, 0, offset, size));

	if (TST_RET) {
		SAFE_CLOSE(fd);

		if (TST_ERR == ENOTSUP)
			tst_brk(TCONF | TTERRNO, "fallocate() not supported");

		tst_brk(TBROK | TTERRNO, "fallocate(fd, 0, %ld, %ld)", offset,
			size);
	}

	if (tc->fill_fs)
		tst_fill_fs(MNTPOINT, 1);

	SAFE_LSEEK(fd, offset, SEEK_SET);
	TEST(write(fd, wbuf, size));
	if (check_result(tc, "write()", size))
		tst_res(TPASS, "Misaligned allocation works as expected");

	/* Deallocation test */
	size = DEALLOCATE_BLOCKS * blocksize;
	offset = block_offset;
	TEST(fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset,
		size));

	if (TST_RET == -1 && TST_ERR == ENOTSUP) {
		tst_res(TCONF | TTERRNO, TESTED_FLAGS);
		goto end;
	}

	if (!check_result(tc, TESTED_FLAGS, 0) || TST_RET)
		goto end;

	/* Validate that fallocate() cleared the correct file range */
	SAFE_LSEEK(fd, 0, SEEK_SET);
	SAFE_READ(1, fd, rbuf, rbuf_size);

	for (err = 0, i = offset; i < offset + size; i++) {
		if (rbuf[i]) {
			err = 1;
			break;
		}
	}

	err = err || memcmp(rbuf, wbuf, offset);
	offset += size;
	size = rbuf_size - offset;
	err = err || memcmp(rbuf + offset, wbuf + offset, size);

	if (err)
		tst_res(TFAIL, TESTED_FLAGS
			" did not clear the correct file range.");
	else
		tst_res(TPASS, TESTED_FLAGS " cleared the correct file range");

end:
	SAFE_CLOSE(fd);
	tst_purge_dir(MNTPOINT);
}

static void cleanup(void)
{
	free(wbuf);
	free(rbuf);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e093c4be760e"},
		{"linux-git", "6d4572a9d71d"},
		{}
	}
};
