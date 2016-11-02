/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by John George
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * umask(2) sets the mask from 0000 to 0777 while we create files,
 * the previous value of the mask should be returned correctly,
 * and the file mode should be correct for each creation mask.
 */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tst_test.h"

static void verify_umask(void)
{
	struct stat statbuf;
	int mskval;
	int fd;
	int failflag = 0;
	unsigned low9mode;

	for (mskval = 0000; mskval < 01000; mskval++) {
		TEST(umask(mskval));
		if (TEST_RETURN < 0 || TEST_RETURN > 0777) {
			tst_brk(TFAIL, "umask(%o) result outside range %ld",
				mskval, TEST_RETURN);
		}

		if (mskval > 0000 && TEST_RETURN != mskval - 1) {
			failflag = 1;
			tst_res(TFAIL, "umask(%o) returned %ld, expected %d",
				mskval, TEST_RETURN, mskval - 1);
		}

		fd = SAFE_CREAT("testfile", 0777);
		SAFE_CLOSE(fd);

		SAFE_STAT("testfile", &statbuf);

		low9mode = statbuf.st_mode & 0777;

		if (low9mode != (~mskval & 0777)) {
			failflag = 1;
			tst_res(TFAIL, "File mode got %o, expected %o",
				low9mode, ~mskval & 0777);
		}

		SAFE_UNLINK("testfile");
	}

	if (!failflag)
		tst_res(TPASS, "All files created with correct mode");
}

static struct tst_test test = {
	.tid = "umask01",
	.test_all = verify_umask,
	.needs_tmpdir = 1,
};
