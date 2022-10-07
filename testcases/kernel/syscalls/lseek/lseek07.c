// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 06/2017 modified by Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * lseek() succeeds to set the specified offset according to whence
 * and write valid data from this location.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define TFILE1 "tfile1"
#define TFILE2 "tfile2"
#define WR_STR1 "abcdefg"
#define WR_STR2 "ijk"

static int fd1, fd2;
static struct tcase {
	int *fd;
	char *fname;
	off_t off;
	off_t exp_off;
	int exp_size;
	char *exp_data;
} tcases[] = {
	{&fd1, TFILE1, 7, 7, 10, "abcdefgijk"},
	{&fd2, TFILE2, 2, 2, 7, "abijkfg"},
};

static void verify_lseek(unsigned int n)
{
	char read_buf[64];
	struct tcase *tc = &tcases[n];

	memset(read_buf, 0, sizeof(read_buf));

	TEST(lseek(*tc->fd, tc->off, SEEK_SET));
	if (TST_RET == (off_t) -1) {
		tst_res(TFAIL | TTERRNO, "lseek(%s, %lld, SEEK_SET) failed",
			tc->fname, (long long int)tc->off);
		return;
	}

	if (TST_RET != tc->exp_off) {
		tst_res(TFAIL, "lseek(%s, %lld, SEEK_SET) returned %ld, expected %lld",
			tc->fname, (long long int)tc->off, TST_RET,
			(long long int)tc->exp_off);
		return;
	}

	SAFE_WRITE(SAFE_WRITE_ALL, *tc->fd, WR_STR2, sizeof(WR_STR2) - 1);

	SAFE_CLOSE(*tc->fd);

	*tc->fd = SAFE_OPEN(tc->fname, O_RDWR);

	SAFE_READ(1, *tc->fd, read_buf, tc->exp_size);

	if (strcmp(read_buf, tc->exp_data)) {
		tst_res(TFAIL, "lseek(%s, %lld, SEEK_SET) wrote incorrect data %s",
			tc->fname, (long long int)tc->off, read_buf);
	} else {
		tst_res(TPASS, "lseek(%s, %lld, SEEK_SET) wrote correct data %s",
			tc->fname, (long long int)tc->off, read_buf);
	}
}

static void setup(void)
{
	fd1 = SAFE_OPEN(TFILE1, O_RDWR | O_CREAT, 0644);
	fd2 = SAFE_OPEN(TFILE2, O_RDWR | O_CREAT, 0644);

	SAFE_WRITE(SAFE_WRITE_ALL, fd1, WR_STR1, sizeof(WR_STR1) - 1);
	SAFE_WRITE(SAFE_WRITE_ALL, fd2, WR_STR1, sizeof(WR_STR1) - 1);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd2 > 0)
		SAFE_CLOSE(fd2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_lseek,
	.needs_tmpdir = 1,
};
