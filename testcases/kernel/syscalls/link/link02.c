// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: William Roske
 *  CO-PILOT		: Dave Fenner
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Tests that link(2) succeeds.
 */

#include <unistd.h>
#include <sys/stat.h>
#include "tst_test.h"

#define OLDPATH "oldpath"
#define NEWPATH "newpath"

static void verify_link(void)
{
	struct stat fbuf, lbuf;

	TST_EXP_PASS(link(OLDPATH, NEWPATH));

	if (!TST_PASS)
		return;

	SAFE_STAT(OLDPATH, &fbuf);
	SAFE_STAT(NEWPATH, &lbuf);

	if (fbuf.st_nlink > 1 && fbuf.st_nlink == lbuf.st_nlink) {
		tst_res(TPASS, "link("OLDPATH","NEWPATH") "
                                 "returned 0 and stat link counts match");
	} else {
		tst_res(TFAIL, "link("OLDPATH","NEWPATH") returned 0"
                                 " but stat link counts do not match %d %d",
                                 (int)fbuf.st_nlink, (int)lbuf.st_nlink);
	}

	SAFE_UNLINK(NEWPATH);
}

static void setup(void)
{
	SAFE_TOUCH(OLDPATH, 0700, NULL);
}

static struct tst_test test = {
	.test_all = verify_link,
	.setup = setup,
	.needs_tmpdir = 1,
};
