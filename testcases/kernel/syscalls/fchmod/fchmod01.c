// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Wayne Boyer and William Roske
 */

/*\
 * Verify that fchmod() can succeed to change the mode permissions of a file specified
 * by file descriptor.
 */

#include <errno.h>

#include "tst_test.h"
#include "fchmod.h"

static int fd;
static int modes[] = { 0, 07, 070, 0700, 0777, 02777, 04777, 06777 };

static void verify_fchmod(void)
{
	struct stat stat_buf;
	int ind;
	mode_t file_mode, mode;

	for (ind = 0; ind < 8; ind++) {
		mode = (mode_t)modes[ind];

		TEST(fchmod(fd, mode));
		if (TST_RET == -1)
			tst_res(TFAIL | TTERRNO, "fchmod() failed unexpectly");

		SAFE_FSTAT(fd, &stat_buf);
		file_mode = stat_buf.st_mode;

		if ((file_mode & ~S_IFREG) != mode) {
			tst_res(TFAIL,
				"%s: Incorrect modes 0%03o, Expected 0%03o",
				TESTFILE, file_mode & ~S_IFREG, mode);
		} else {
			tst_res(TPASS,
				"Functionality of fchmod(%d, %#o) successful",
				fd, mode);
		}
	}
}

static void setup(void)
{
	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_fchmod,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
