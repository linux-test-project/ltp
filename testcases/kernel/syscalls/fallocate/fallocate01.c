// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Author: Sharyathi Nagesh <sharyathi@in.ibm.com>
 * Copyright (c) Linux Test Project, 2008-2024
 */

/*\
 * Basic test for :manpage:`fallocate(2)` covering the two preallocation
 * modes and their effect on the file size.
 *
 * A file is pre-populated to 12 blocks and a single extra block is
 * preallocated past the end of the file. The test verifies that the
 * resulting file size matches the mode semantics and that the newly
 * allocated region is writable.
 *
 * [Algorithm]
 *
 * - Populate the working file with 12 blocks of block_size bytes.
 * - Preallocate one block at the end of the file.
 * - In DEFAULT_MODE the file grows to 13 blocks.
 * - In FALLOC_FL_KEEP_SIZE mode the file size stays at 12 blocks.
 * - Seek into the newly allocated region and write a byte to it.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fallocate.h"

#define MNTPOINT "mntpoint"
#define FNAME MNTPOINT "/tfile"
#define BLOCKS_WRITTEN 12

static int fd = -1;
static int block_size;

static struct tcase {
	int mode;
	int expected_blocks;
	const char *desc;
} tcases[] = {
	{0, BLOCKS_WRITTEN + 1, "DEFAULT_MODE"},
	{FALLOC_FL_KEEP_SIZE, BLOCKS_WRITTEN, "FALLOC_FL_KEEP_SIZE"},
};

static void populate_file(void)
{
	char buf[block_size];

	for (int blocks = 0; blocks < BLOCKS_WRITTEN; blocks++) {
		for (int i = 0; i < block_size; i++)
			buf[i] = 'A' + (i % 26);

		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, block_size);
	}
}

static void setup(void)
{
	struct stat file_stat;

	TEST(fallocate(-1, 0, 0, 0));
	if (TST_ERR == ENOSYS)
		tst_brk(TCONF, "fallocate() not supported");

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0700);

	SAFE_FSTAT(fd, &file_stat);
	block_size = (int)file_stat.st_blksize;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct stat file_stat;
	loff_t offset, len, pos, write_offset, expected_size;

	/* Reset the backing file to a pristine 12-block state per run. */
	SAFE_FTRUNCATE(fd, 0);
	SAFE_LSEEK(fd, 0, SEEK_SET);
	populate_file();

	offset = SAFE_LSEEK(fd, 0, SEEK_END);
	len = block_size;
	expected_size = (loff_t)tc->expected_blocks * block_size;

	TEST(fallocate(fd, tc->mode, offset, len));
	if (TST_RET != 0) {
		if (TST_ERR == EOPNOTSUPP) {
			tst_res(TCONF | TTERRNO,
				"fallocate(%s, %lld, %lld) not supported",
				tc->desc, (long long)offset, (long long)len);
			return;
		}

		tst_res(TFAIL | TTERRNO,
			"fallocate(%s, %lld, %lld) failed",
			tc->desc, (long long)offset, (long long)len);
		return;
	}

	tst_res(TPASS, "fallocate(%s, %lld, %lld) passed",
		tc->desc, (long long)offset, (long long)len);

	SAFE_FSTAT(fd, &file_stat);
	TST_EXP_EQ_LI(file_stat.st_size, expected_size);

	write_offset = len / 2;
	pos = SAFE_LSEEK(fd, write_offset, SEEK_CUR);
	TST_EXP_EQ_LI(pos, offset + write_offset);

	TST_EXP_POSITIVE(write(fd, "A", 1),
			 "write into the newly allocated region");
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
