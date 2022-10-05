// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 */
/*
 * Description:
 *
 * For each of SEEK_SET, SEEK_CUR and SEEK_END verify that,
 *
 *   1. llseek() succeeds to set file position in the middle of the data. The
 *      file offset is checked by reading from a file and comparing the data.
 *
 *   2. llseek() succeeds to set file postion to the end of the data, reading
 *      this postion returns 0.
 *
 *   3. llseek() succeeds to set file position after the end of the data,
 *      reading from this postion returns 0 as well.
 */
#define _GNU_SOURCE

#include "tst_test.h"

#define TEST_FILE "testfile"
#define STR "abcdefgh"

static void setup(void)
{
	int fd;

	fd = SAFE_CREAT(TEST_FILE, 0644);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, STR, sizeof(STR) - 1);

	SAFE_CLOSE(fd);
}

static struct tcase {
	int whence;
	int off;
	int ret;
	int read;
	const char *str;
} tcases[] = {
	/* Seek somewhere in the middle of data */
	{SEEK_SET, 1, 1, 3, "bcd"},
	{SEEK_CUR, 1, 5, 3, "fgh"},
	{SEEK_END, -1, 7, 1, "h"},

	/* Seek to the end of data */
	{SEEK_SET, 8, 8, 0, NULL},
	{SEEK_CUR, 4, 8, 0, NULL},
	{SEEK_END, 0, 8, 0, NULL},

	/* Seek after the end of data */
	{SEEK_SET, 10, 10, 0, NULL},
	{SEEK_CUR, 8, 12, 0, NULL},
	{SEEK_END, 4, 12, 0, NULL},
};

static const char *str_whence(int whence)
{
	switch (whence) {
	case SEEK_SET:
		return "SEEK_SET";
	case SEEK_CUR:
		return "SEEK_CUR";
	case SEEK_END:
		return "SEEK_END";
	default:
		return "INVALID";
	}
}

static void verify_lseek64(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	char read_buf[128];
	int fd, ret;

	fd = SAFE_OPEN(TEST_FILE, O_RDONLY);

	SAFE_READ(1, fd, read_buf, 4);

	TEST(lseek64(fd, tc->off, tc->whence));

	if (TST_RET == -1) {
                tst_res(TFAIL | TTERRNO, "llseek failed on %s ", TEST_FILE);
                goto exit;
        }

	if (TST_RET != tc->ret) {
                tst_res(TFAIL, "llseek returned %li expected %i", TST_RET, tc->ret);
                goto exit;
        } else {
		tst_res(TPASS, "llseek returned %i", tc->ret);
	}

        memset(read_buf, 0, sizeof(read_buf));

        ret = SAFE_READ(0, fd, read_buf, tc->read);

	if (!tc->read) {
		if (ret != 0)
			tst_res(TFAIL, "Read bytes after llseek to end of file");
		else
			tst_res(TPASS, "%s read returned 0", str_whence(tc->whence));

		goto exit;
	}

        if (strcmp(read_buf, tc->str))
                tst_res(TFAIL, "Read wrong bytes after llseek");
        else
                tst_res(TPASS, "%s for llseek", str_whence(tc->whence));

exit:
        SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test = verify_lseek64,
	.tcnt = ARRAY_SIZE(tcases),
};
