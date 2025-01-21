// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Test allocates a file with specified size then tests the following modes:
 * FALLOC_FL_PUNCH_HOLE, FALLOC_FL_ZERO_RANGE and FALLOC_FL_COLLAPSE_RANGE.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tst_test.h"
#include "lapi/fallocate.h"

static int fd;
static size_t block_size;
static size_t buf_size;

#define MNTPOINT "fallocate"
#define FNAME MNTPOINT "/fallocate.txt"
#define NUM_OF_BLOCKS	3

static char *verbose;

static void get_blocksize(void)
{
	struct stat file_stat;

	SAFE_FSTAT(fd, &file_stat);

	block_size = file_stat.st_blksize;
	buf_size = NUM_OF_BLOCKS * block_size;
}

static size_t get_allocsize(void)
{
	struct stat file_stat;

	fsync(fd);

	SAFE_FSTAT(fd, &file_stat);

	return file_stat.st_blocks * 512;
}

static void fill_tst_buf(char buf[])
{
	/* fill the buffer with a, b, c, ... letters on each block */
	int i;

	for (i = 0; i < NUM_OF_BLOCKS; ++i)
		memset(buf + i * block_size, 'a' + i, block_size);
}

static void check_file_data(const char exp_buf[], size_t size)
{
	char rbuf[size];

	tst_res(TINFO, "reading the file, compare with expected buffer");

	SAFE_LSEEK(fd, 0, SEEK_SET);
	SAFE_READ(1, fd, rbuf, size);

	if (memcmp(exp_buf, rbuf, size)) {
		if (verbose) {
			tst_res_hexd(TINFO, exp_buf, size, "expected:");
			tst_res_hexd(TINFO, rbuf, size, "but read:");
		}
		tst_brk(TFAIL, "not expected file data");
	}
}

static void test01(void)
{
	tst_res(TINFO, "allocate '%zu' bytes", buf_size);

	if (fallocate(fd, 0, 0, buf_size) == -1) {
		if (errno == ENOSYS || errno == EOPNOTSUPP)
			tst_brk(TCONF, "fallocate() not supported");
		tst_brk(TFAIL | TERRNO, "fallocate() failed");
	}

	char buf[buf_size];

	fill_tst_buf(buf);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, buf_size);

	tst_res(TPASS, "test-case succeeded");
}

static void test02(void)
{
	size_t alloc_size0 = get_allocsize();

	tst_res(TINFO, "read allocated file size '%zu'", alloc_size0);
	tst_res(TINFO, "make a hole with FALLOC_FL_PUNCH_HOLE");

	if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
	    block_size, block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
			        "FALLOC_FL_PUNCH_HOLE not supported");
		}
		tst_brk(TFAIL | TERRNO, "fallocate() failed");
	}

	tst_res(TINFO, "check that file has a hole with lseek(,,SEEK_HOLE)");
	off_t ret = lseek(fd, 0, SEEK_HOLE);

	if (ret != (ssize_t)block_size) {
		/* exclude error when kernel doesn't have SEEK_HOLE support */
		if (errno != EINVAL) {
			tst_brk(TFAIL | TERRNO,
				 "fallocate() or lseek() failed");
		}
		tst_brk(TBROK | TERRNO,
			"lseek() doesn't support SEEK_HOLE");
	} else {
		tst_res(TINFO, "found a hole at '%ld' offset", ret);
	}

	size_t alloc_size1 = get_allocsize();

	tst_res(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 - block_size) != alloc_size1)
		tst_brk(TFAIL, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size, 0, block_size);

	check_file_data(exp_buf, buf_size);

	tst_res(TPASS, "test-case succeeded");
}

static void test03(void)
{
	tst_res(TINFO, "zeroing file space with FALLOC_FL_ZERO_RANGE");

	size_t alloc_size0 = get_allocsize();

	tst_res(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_ZERO_RANGE, block_size - 1,
	    block_size + 2) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
			        "FALLOC_FL_ZERO_RANGE not supported");
		}
		tst_brk(TFAIL | TERRNO, "fallocate failed");
	}

	/* The file hole in the specified range must be allocated and
	 * filled with zeros. Check it.
	 */
	size_t alloc_size1 = get_allocsize();

	tst_res(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 + block_size) != alloc_size1)
		tst_brk(TFAIL, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size - 1, 0, block_size + 2);

	check_file_data(exp_buf, buf_size);

	tst_res(TPASS, "test-case succeeded");
}

static void test04(void)
{
	tst_res(TINFO, "collapsing file space with FALLOC_FL_COLLAPSE_RANGE");

	size_t alloc_size0 = get_allocsize();

	tst_res(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_COLLAPSE_RANGE, block_size,
	    block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
			        "FALLOC_FL_COLLAPSE_RANGE not supported");
		}
		tst_brk(TFAIL | TERRNO, "fallocate failed");
	}

	size_t alloc_size1 = get_allocsize();

	tst_res(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 - block_size) != alloc_size1)
		tst_brk(TFAIL, "not expected allocated size");

	size_t size = buf_size - block_size;
	char tmp_buf[buf_size];
	char exp_buf[size];

	fill_tst_buf(tmp_buf);

	memcpy(exp_buf, tmp_buf, block_size);
	memcpy(exp_buf + block_size, tmp_buf + 2 * block_size,
	       buf_size - block_size * 2);

	exp_buf[block_size - 1] = exp_buf[block_size] = '\0';
	check_file_data(exp_buf, size);

	tst_res(TPASS, "test-case succeeded");
}

static void test05(void)
{
	tst_res(TINFO, "inserting space with FALLOC_FL_INSERT_RANGE");

	size_t alloc_size0 = get_allocsize();

	tst_res(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_INSERT_RANGE, block_size,
	    block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF,
				"FALLOC_FL_INSERT_RANGE not supported");
		}
		tst_brk(TFAIL | TERRNO, "fallocate failed");
	}

	/* allocate space and ensure that it filled with zeroes */
	if (fallocate(fd, FALLOC_FL_ZERO_RANGE, block_size, block_size) == -1)
		tst_brk(TFAIL | TERRNO, "fallocate failed");

	size_t alloc_size1 = get_allocsize();

	tst_res(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 + block_size) != alloc_size1)
		tst_brk(TFAIL, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size - 1, 0, block_size + 2);

	check_file_data(exp_buf, buf_size);

	tst_res(TPASS, "test-case succeeded");
}

static void (*tcases[])(void) = {
	test01, test02, test03, test04, test05
};

static void run(unsigned int i)
{
	tcases[i]();
}

static void setup(void)
{
	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0700);

	get_blocksize();
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.timeout = 9,
	.options = (struct tst_option[]) {
		{"v", &verbose, "Turns on verbose mode"},
		{}
	},
	.cleanup = cleanup,
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.needs_root = 1,
};
