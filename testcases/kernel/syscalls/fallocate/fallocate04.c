/*
 * Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Description:
 * Test allocates a file with specified size then tests the following modes:
 * FALLOC_FL_PUNCH_HOLE, FALLOC_FL_ZERO_RANGE and FALLOC_FL_COLLAPSE_RANGE.
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fallocate.h"

char *TCID = "fallocate04";
int TST_TOTAL = 5;

static int fd;
static const char fname[] = "fallocate04.txt";
static size_t block_size;
static size_t buf_size;

#define NUM_OF_BLOCKS	3

static int verbose;
static const option_t options[] = {
	{"v", &verbose, NULL},
	{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -v      Verbose\n");
}

static void cleanup(void)
{
	close(fd);
	tst_rmdir();
}

static void get_blocksize(void)
{
	struct stat file_stat;

	SAFE_FSTAT(cleanup, fd, &file_stat);

	block_size = file_stat.st_blksize;
	buf_size = NUM_OF_BLOCKS * block_size;
}

static size_t get_allocsize(void)
{
	struct stat file_stat;

	fsync(fd);

	SAFE_FSTAT(cleanup, fd, &file_stat);

	return file_stat.st_blocks * 512;
}

static void fill_tst_buf(char buf[])
{
	/* fill the buffer with a, b, c, ... letters on each block */
	int i;

	for (i = 0; i < NUM_OF_BLOCKS; ++i)
		memset(buf + i * block_size, 'a' + i, block_size);
}

static void setup(void)
{
	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0700);

	get_blocksize();
}

static void check_file_data(const char exp_buf[], size_t size)
{
	char rbuf[size];

	tst_resm(TINFO, "reading the file, compare with expected buffer");

	SAFE_LSEEK(cleanup, fd, 0, SEEK_SET);
	SAFE_READ(cleanup, 1, fd, rbuf, size);

	if (memcmp(exp_buf, rbuf, size)) {
		if (verbose) {
			tst_resm_hexd(TINFO, exp_buf, size, "expected:");
			tst_resm_hexd(TINFO, rbuf, size, "but read:");
		}
		tst_brkm(TFAIL, cleanup, "not expected file data");
	}
}

static void test01(void)
{
	tst_resm(TINFO, "allocate '%zu' bytes", buf_size);

	if (fallocate(fd, 0, 0, buf_size) == -1) {
		if (errno == ENOSYS || errno == EOPNOTSUPP)
			tst_brkm(TCONF, cleanup, "fallocate() not supported");
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate() failed");
	}

	char buf[buf_size];

	fill_tst_buf(buf);

	SAFE_WRITE(cleanup, 1, fd, buf, buf_size);

	tst_resm(TPASS, "test-case succeeded");
}

static void test02(void)
{
	size_t alloc_size0 = get_allocsize();

	tst_resm(TINFO, "read allocated file size '%zu'", alloc_size0);
	tst_resm(TINFO, "make a hole with FALLOC_FL_PUNCH_HOLE");

	if (tst_kvercmp(2, 6, 38) < 0) {
		tst_brkm(TCONF, cleanup,
			 "FALLOC_FL_PUNCH_HOLE needs Linux 2.6.38 or newer");
	}

	if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
	    block_size, block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brkm(TCONF, cleanup,
			         "FALLOC_FL_PUNCH_HOLE not supported");
		}
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate() failed");
	}

	tst_resm(TINFO, "check that file has a hole with lseek(,,SEEK_HOLE)");
	off_t ret = lseek(fd, 0, SEEK_HOLE);

	if (ret != (ssize_t)block_size) {
		/* exclude error when kernel doesn't have SEEK_HOLE support */
		if (errno != EINVAL) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "fallocate() or lseek() failed");
		}
		if (tst_kvercmp(3, 1, 0) < 0) {
			tst_resm(TINFO, "lseek() doesn't support SEEK_HOLE, "
				 "this is expected for < 3.1 kernels");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "lseek() doesn't support SEEK_HOLE");
		}
	} else {
		tst_resm(TINFO, "found a hole at '%ld' offset", ret);
	}

	size_t alloc_size1 = get_allocsize();

	tst_resm(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 - block_size) != alloc_size1)
		tst_brkm(TFAIL, cleanup, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size, 0, block_size);

	check_file_data(exp_buf, buf_size);

	tst_resm(TPASS, "test-case succeeded");
}

static void test03(void)
{
	tst_resm(TINFO, "zeroing file space with FALLOC_FL_ZERO_RANGE");

	if (tst_kvercmp(3, 15, 0) < 0) {
		tst_brkm(TCONF, cleanup,
			 "FALLOC_FL_ZERO_RANGE needs Linux 3.15 or newer");
	}

	size_t alloc_size0 = get_allocsize();

	tst_resm(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_ZERO_RANGE, block_size - 1,
	    block_size + 2) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brkm(TCONF, cleanup,
			         "FALLOC_FL_ZERO_RANGE not supported");
		}
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate failed");
	}

	/* The file hole in the specified range must be allocated and
	 * filled with zeros. Check it.
	 */
	size_t alloc_size1 = get_allocsize();

	tst_resm(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 + block_size) != alloc_size1)
		tst_brkm(TFAIL, cleanup, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size - 1, 0, block_size + 2);

	check_file_data(exp_buf, buf_size);

	tst_resm(TPASS, "test-case succeeded");
}

static void test04(void)
{
	tst_resm(TINFO, "collapsing file space with FALLOC_FL_COLLAPSE_RANGE");

	size_t alloc_size0 = get_allocsize();

	tst_resm(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_COLLAPSE_RANGE, block_size,
	    block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brkm(TCONF, cleanup,
			         "FALLOC_FL_COLLAPSE_RANGE not supported");
		}
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate failed");
	}

	size_t alloc_size1 = get_allocsize();

	tst_resm(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 - block_size) != alloc_size1)
		tst_brkm(TFAIL, cleanup, "not expected allocated size");

	size_t size = buf_size - block_size;
	char tmp_buf[buf_size];
	char exp_buf[size];

	fill_tst_buf(tmp_buf);

	memcpy(exp_buf, tmp_buf, block_size);
	memcpy(exp_buf + block_size, tmp_buf + 2 * block_size,
	       buf_size - block_size * 2);

	exp_buf[block_size - 1] = exp_buf[block_size] = '\0';
	check_file_data(exp_buf, size);

	tst_resm(TPASS, "test-case succeeded");
}

static void test05(void)
{
	tst_resm(TINFO, "inserting space with FALLOC_FL_INSERT_RANGE");

	size_t alloc_size0 = get_allocsize();

	tst_resm(TINFO, "read current allocated file size '%zu'", alloc_size0);

	if (fallocate(fd, FALLOC_FL_INSERT_RANGE, block_size,
	    block_size) == -1) {
		if (errno == EOPNOTSUPP) {
			tst_brkm(TCONF, cleanup,
				 "FALLOC_FL_INSERT_RANGE not supported");
		}
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate failed");
	}

	/* allocate space and ensure that it filled with zeroes */
	if (fallocate(fd, FALLOC_FL_ZERO_RANGE, block_size, block_size) == -1)
		tst_brkm(TFAIL | TERRNO, cleanup, "fallocate failed");

	size_t alloc_size1 = get_allocsize();

	tst_resm(TINFO, "allocated file size before '%zu' and after '%zu'",
		 alloc_size0, alloc_size1);
	if ((alloc_size0 + block_size) != alloc_size1)
		tst_brkm(TFAIL, cleanup, "not expected allocated size");

	char exp_buf[buf_size];

	fill_tst_buf(exp_buf);
	memset(exp_buf + block_size - 1, 0, block_size + 2);

	check_file_data(exp_buf, buf_size);

	tst_resm(TPASS, "test-case succeeded");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, help);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		test01();
		test02();
		test03();
		test04();
		test05();
	}

	cleanup();

	tst_exit();
}
