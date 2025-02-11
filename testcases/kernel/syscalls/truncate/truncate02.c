// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 John George
 */

/*\
 * Verify that:
 *
 * - truncate(2) truncates a file to a specified length successfully.
 * - If the file is larger than the specified length, the extra data is lost.
 * - If the file is shorter than the specified length, the extra data is filled by '0'.
 * - truncate(2) doesn't change offset.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "tst_test.h"
#include "tst_safe_prw.h"

#define TESTFILE	"testfile"
#define FILE_SIZE	1024
#define TRUNC_LEN1	256
#define TRUNC_LEN2	512

static int fd;

static struct tcase {
	off_t trunc_len;
	off_t read_off;
	off_t read_count;
	char exp_char;
} tcases[] = {
	{TRUNC_LEN1, 0, TRUNC_LEN1, 'a'},
	{TRUNC_LEN2, TRUNC_LEN1, TRUNC_LEN1, '\0'},
};

static void verify_truncate(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct stat stat_buf;
	char read_buf[tc->read_count];
	int i;

	memset(read_buf, 'b', tc->read_count);

	TEST(truncate(TESTFILE, tc->trunc_len));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "truncate(%s, %ld) failed",
			TESTFILE, tc->trunc_len);
		return;
	}

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"truncate(%s, %ld) returned invalid value %ld",
			TESTFILE, tc->trunc_len, TST_RET);
		return;
	}

	SAFE_STAT(TESTFILE, &stat_buf);
	if (stat_buf.st_size != tc->trunc_len) {
		tst_res(TFAIL, "%s: Incorrect file size %ld, expected %ld",
			TESTFILE, stat_buf.st_size, tc->trunc_len);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR)) {
		tst_res(TFAIL, "truncate(%s, %ld) changes offset",
			TESTFILE, tc->trunc_len);
		return;
	}

	SAFE_PREAD(1, fd, read_buf, tc->read_count, tc->read_off);
	for (i = 0; i < tc->read_count; i++) {
		if (read_buf[i] != tc->exp_char) {
			tst_res(TFAIL, "%s: wrong content %c, expected %c",
				TESTFILE, read_buf[i], tc->exp_char);
			return;
		}
	}

	tst_res(TPASS, "truncate(%s, %ld) succeeded",
		TESTFILE, tc->trunc_len);
}

static void setup(void)
{
	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, 0644);

	tst_fill_fd(fd, 'a', FILE_SIZE, 1);

	SAFE_LSEEK(fd, 0, SEEK_SET);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_truncate,
};
