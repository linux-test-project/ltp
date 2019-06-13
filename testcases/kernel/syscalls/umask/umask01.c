// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *  07/2001 Ported by John George
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
		if (TST_RET < 0 || TST_RET > 0777) {
			tst_brk(TFAIL, "umask(%o) result outside range %ld",
				mskval, TST_RET);
		}

		if (mskval > 0000 && TST_RET != mskval - 1) {
			failflag = 1;
			tst_res(TFAIL, "umask(%o) returned %ld, expected %d",
				mskval, TST_RET, mskval - 1);
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
	.test_all = verify_umask,
	.needs_tmpdir = 1,
};
