/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 06/2017 modified by Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Description:
 *  lseek() succeeds to set the specified offset according to whence
 *  and read valid data from this location.
 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define WRITE_STR "abcdefg"
#define TFILE "tfile"

static int fd;
static struct tcase {
	off_t off;
	int whence;
	char *wname;
	off_t exp_off;
	ssize_t exp_size;
	char *exp_data;
} tcases[] = {
	{4, SEEK_SET, "SEEK_SET", 4, 3, "efg"},
	{-2, SEEK_CUR, "SEEK_CUR", 5, 2, "fg"},
	{-4, SEEK_END, "SEEK_END", 3, 4, "defg"},
	{0, SEEK_END, "SEEK_END", 7, 0, NULL},
};

static void verify_lseek(unsigned int n)
{
	char read_buf[64];
	struct tcase *tc = &tcases[n];

	// reset the offset to end of file
	SAFE_READ(0, fd, read_buf, sizeof(read_buf));

	memset(read_buf, 0, sizeof(read_buf));

	TEST(lseek(fd, tc->off, tc->whence));
	if (TEST_RETURN == (off_t) -1) {
		tst_res(TFAIL | TTERRNO, "lseek(%s, %ld, %s) failed", TFILE,
			tc->off, tc->wname);
		return;
	}

	if (TEST_RETURN != tc->exp_off) {
		tst_res(TFAIL, "lseek(%s, %ld, %s) returned %ld, expected %ld",
			TFILE, tc->off, tc->wname, TEST_RETURN, tc->exp_off);
		return;
	}

	SAFE_READ(1, fd, read_buf, tc->exp_size);

	if (tc->exp_data && strcmp(read_buf, tc->exp_data)) {
		tst_res(TFAIL, "lseek(%s, %ld, %s) read incorrect data",
			TFILE, tc->off, tc->wname);
	} else {
		tst_res(TPASS, "lseek(%s, %ld, %s) read correct data",
			TFILE, tc->off, tc->wname);
	}
}

static void setup(void)
{
	fd = SAFE_OPEN(TFILE, O_RDWR | O_CREAT, 0644);

	SAFE_WRITE(1, fd, WRITE_STR, sizeof(WRITE_STR) - 1);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_lseek,
	.needs_tmpdir = 1,
};
