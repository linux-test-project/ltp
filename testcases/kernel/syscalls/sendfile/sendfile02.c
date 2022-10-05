// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 08/2002 Make it use a socket so it works with 2.5 kernel
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Test the basic functionality of the sendfile() system call:
 *
 * 1. Call sendfile() with offset = 0.
 * 2. Call sendfile() with offset in the middle of the file.
 */

#include <stdio.h>
#include <inttypes.h>
#include <sys/sendfile.h>

#include "tst_test.h"

#define IN_FILE "in_file"
#define OUT_FILE "out_file"

#define OFFSET_DESC(x) .desc = "with offset = "#x, .offset = x

struct test_case_t {
	char *desc;
	off_t offset;
	int64_t count;
	int64_t exp_retval;
	int64_t exp_updated_offset;
} tc[] = {
	{ OFFSET_DESC(0), 26, 26, 26 },
	{ OFFSET_DESC(2), 24, 24, 26 },
};

static void setup(void)
{
	int fd;
	char buf[27];

	fd = SAFE_CREAT(IN_FILE, 00700);
	sprintf(buf, "abcdefghijklmnopqrstuvwxyz");
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, strlen(buf));
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

	if (tc[i].exp_retval != TST_RET)
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
};
