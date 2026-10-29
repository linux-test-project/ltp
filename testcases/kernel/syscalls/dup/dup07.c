// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Ported from SPIE, section2/iosuite/dup3.c, by Airong Zhang
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2006-2024
 */

/*\
 * Verify that the file descriptor created by dup(2) syscall has the same
 * access mode as the old one.
 */

#include "tst_test.h"

static const char *testfile = "dup07";

static struct tcase {
	char *mode_desc;
	int mode;
} tcases[] = {
	{"read only", 0444},
	{"write only", 0222},
	{"read/write", 0666},
};

static void run(unsigned int n)
{
	int oldfd, dupfd;
	struct stat oldbuf, dupbuf;
	struct tcase *tc = &tcases[n];

	oldfd = SAFE_CREAT(testfile, tc->mode);
	dupfd = TST_EXP_FD_SILENT(dup(oldfd), "dup() %s file", tc->mode_desc);
	if (TST_PASS) {
		SAFE_FSTAT(oldfd, &oldbuf);
		SAFE_FSTAT(dupfd, &dupbuf);

		if (oldbuf.st_mode != dupbuf.st_mode)
			tst_res(TFAIL, "%s and dup do not match", tc->mode_desc);
		else
			tst_res(TPASS, "Passed in %s mode", tc->mode_desc);

		SAFE_CLOSE(dupfd);
	}

	SAFE_CLOSE(oldfd);
	SAFE_UNLINK(testfile);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
