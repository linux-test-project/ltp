// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Red Hat Inc., 2007
 * 11/2007 Copyed from sendfile02.c by Masatake YAMATO
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Test that sendfile() system call updates file position of in_fd correctly
 * when passing NULL as offset.
 */

#include <stdio.h>
#include <inttypes.h>
#include <sys/sendfile.h>

#include "tst_test.h"

#define IN_FILE "in_file"
#define OUT_FILE "out_file"

static struct stat sb;

static void setup(void)
{
	int fd;
	char buf[27];

	fd = SAFE_CREAT(IN_FILE, 00700);
	sprintf(buf, "abcdefghijklmnopqrstuvwxyz");
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, strlen(buf));
	SAFE_FSTAT(fd, &sb);
	SAFE_CLOSE(fd);

	fd = SAFE_CREAT(OUT_FILE, 00700);
	SAFE_CLOSE(fd);
}

static void run(void)
{
	off_t after_pos;
	int in_fd = SAFE_OPEN(IN_FILE, O_RDONLY);
	int out_fd = SAFE_OPEN(OUT_FILE, O_WRONLY);

	TEST(sendfile(out_fd, in_fd, NULL, sb.st_size));
	after_pos = SAFE_LSEEK(in_fd, 0, SEEK_CUR);

	if (sb.st_size != TST_RET)
		tst_res(TFAIL, "sendfile() failed to return expected value, expected: %"
			PRId64 ", got: %ld",
			sb.st_size, TST_RET);
	else if (after_pos != sb.st_size)
		tst_res(TFAIL, "sendfile() updated the file position of in_fd unexpectedly,"
			" expected file position: %" PRId64
			" actual file position %" PRId64,
			(int64_t)(sb.st_size), (int64_t)(after_pos));
	else
		tst_res(TPASS, "sendfile() with offset=NULL");

	SAFE_CLOSE(in_fd);
	SAFE_CLOSE(out_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = run,
};
