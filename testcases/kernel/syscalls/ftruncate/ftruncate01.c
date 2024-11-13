// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Wayne Boyer
 */

/*\
 * [Description]
 *
 * Verify that, ftruncate() succeeds to truncate a file to a certain length,
 * if the file previously is smaller than the truncated size, ftruncate()
 * shall increase the size of the file.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"

#define TESTFILE	"testfile"

#define TRUNC_LEN1	256
#define TRUNC_LEN2	512
#define FILE_SIZE	1024

static int fd;

static void check_and_report(off_t offset, char data, off_t trunc_len)
{
	int i, file_length;
	char buf[FILE_SIZE];
	struct stat stat_buf;

	memset(buf, '*', sizeof(buf));

	SAFE_FSTAT(fd, &stat_buf);
	file_length = stat_buf.st_size;

	if (file_length != trunc_len) {
		tst_res(TFAIL, "ftruncate() got incorrected size: %d",
			file_length);
		return;
	}

	SAFE_LSEEK(fd, offset, SEEK_SET);
	SAFE_READ(0, fd, buf, sizeof(buf));

	for (i = 0; i < TRUNC_LEN1; i++) {
		if (buf[i] != data) {
			tst_res(TFAIL,
				"ftruncate() got incorrect data %i, expected %i",
				buf[i], data);
			return;
		}
	}

	tst_res(TPASS, "ftruncate() succeeded");
}

static void verify_ftruncate(void)
{
	tst_res(TINFO, "Truncated length smaller than file size");
	TEST(ftruncate(fd, TRUNC_LEN1));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "ftruncate() failed");
		return;
	}

	check_and_report(0, 'a', TRUNC_LEN1);

	tst_res(TINFO, "Truncated length exceeds file size");
	TEST(ftruncate(fd, TRUNC_LEN2));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "ftruncate() failed");
		return;
	}

	check_and_report(TRUNC_LEN1, 0, TRUNC_LEN2);
}

static void setup(void)
{
	if (tst_fill_file(TESTFILE, 'a', FILE_SIZE, 1))
		tst_brk(TBROK, "Failed to create test file");

	fd = SAFE_OPEN(TESTFILE, O_RDWR);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_ftruncate,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
