// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2014
 */

/*\
 * [Description]
 *
 * Testcase copied from sendfile02.c to test the basic functionality of
 * the sendfile() system call on large file. There is a kernel bug which
 * introduced by commit 8f9c0119d7ba9 and fixed by commit 5d73320a96fcc.
 *
 * Only supports 64bit systems.
 *
 * [Algorithm]
 *
 * 1. Call sendfile() with offset at 0.
 * 2. Call sendfile() with offset at 3GB.
 */

#include <inttypes.h>
#include <sys/sendfile.h>

#include "tst_test.h"
#include "lapi/abisize.h"

#ifndef TST_ABI32

#define ONE_GB		(INT64_C(1) << 30)
#define IN_FILE		"in_file"
#define OUT_FILE	"out_file"

static struct test_case_t {
	char *desc;
	off_t offset;
	int64_t count;
	int64_t exp_retval;
	int64_t exp_updated_offset;
} tc[] = {
	{ "offset at 0", 0, ONE_GB, ONE_GB, ONE_GB },
	{ "offset at 3GB", 3 * ONE_GB, ONE_GB, ONE_GB, 4 * ONE_GB }
};

static void setup(void)
{
	int i, fd;

	if (!tst_fs_has_free(".", 5, TST_GB))
		tst_brk(TCONF, "Test on large file needs 5G free space");

	fd = SAFE_CREAT(IN_FILE, 00700);
	for (i = 1; i <= (4 * 1024); ++i) {
		SAFE_LSEEK(fd, 1024 * 1024 - 1, SEEK_CUR);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "C", 1);
	}
	SAFE_CLOSE(fd);

	fd = SAFE_CREAT(OUT_FILE, 00700);
	SAFE_CLOSE(fd);
}

static void run(unsigned int i)
{
	int in_fd = SAFE_OPEN(IN_FILE, O_RDONLY);
	int out_fd = SAFE_OPEN(OUT_FILE, O_WRONLY);
	off_t offset = tc[i].offset;

	off_t before_pos, after_pos;
	before_pos = SAFE_LSEEK(in_fd, 0, SEEK_CUR);

	TEST(sendfile(out_fd, in_fd, &offset, tc[i].count));
	after_pos = SAFE_LSEEK(in_fd, 0, SEEK_CUR);

	if (TST_RET != tc[i].exp_retval)
		tst_res(TFAIL, "sendfile() failed to return expected value, "
			       "expected: %" PRId64 ", got: %ld",
			tc[i].exp_retval, TST_RET);
	else if (offset != tc[i].exp_updated_offset)
		tst_res(TFAIL, "sendfile() failed to update OFFSET parameter to "
			       "expected value, expected: %" PRId64 ", got: %" PRId64,
			tc[i].exp_updated_offset, (int64_t)(offset));
	else if (before_pos != after_pos)
		tst_res(TFAIL, "sendfile() updated the file position of in_fd "
			       "unexpectedly, expected file position: %" PRId64
			       ", actual file position %" PRId64,
			(int64_t)(before_pos), (int64_t)(after_pos));
	else
		tst_res(TPASS, "sendfile() with %s", tc[i].desc);

	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tc),
	.max_runtime = 120,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "5d73320a96fcc"},
		{}
	}
};

#else
TST_TEST_TCONF("This test is only for 64bit");
#endif
