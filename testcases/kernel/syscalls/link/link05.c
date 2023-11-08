// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: Richard Logan, William Roske
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2001-2023
 */

/*\
 * [Description]
 *
 * Tests that link(2) succeeds with creating 1000 links.
 */

#include <stdio.h>
#include <sys/stat.h>
#include "tst_test.h"

#define BASENAME	"lkfile"
#define NLINKS	1000

static char fname[255];

static void verify_link(void)
{
	int cnt = 0, created;
	char lname[1024];
	struct stat fbuf, lbuf;

	for (created = 1; created < NLINKS; created++) {
		sprintf(lname, "%s_%d", fname, created);
		TST_EXP_PASS_SILENT(link(fname, lname), "%d: link(%s, %s)", created,
							fname, lname);
		if (!TST_PASS)
			goto cleanup;
	}

	SAFE_STAT(fname, &fbuf);

	for (cnt = 1; cnt < NLINKS; cnt++) {
		sprintf(lname, "%s_%d", fname, cnt);

		SAFE_STAT(lname, &lbuf);
		if (fbuf.st_nlink <= 1 || lbuf.st_nlink <= 1 ||
		    (fbuf.st_nlink != lbuf.st_nlink)) {

			tst_res(TFAIL,
				 "link(%s, %s[1-%d]) ret %ld for %d files, stat values do not match %d %d",
				 fname, fname, NLINKS, TST_RET, NLINKS,
				 (int)fbuf.st_nlink, (int)lbuf.st_nlink);
			break;
		}
	}

	if (cnt == NLINKS) {
		tst_res(TPASS,
			 "link(%s, %s[1-%d]) ret %ld for %d files, stat linkcounts match %d",
			 fname, fname, NLINKS, TST_RET, NLINKS, (int)fbuf.st_nlink);
	}

cleanup:
	for (cnt = 1; cnt < created; cnt++) {
		sprintf(lname, "%s_%d", fname, cnt);
		SAFE_UNLINK(lname);
	}
}

static void setup(void)
{
	sprintf(fname, "%s_%d", BASENAME, getpid());
	SAFE_TOUCH(fname, 0700, NULL);
}

static struct tst_test test = {
	.test_all = verify_link,
	.setup = setup,
	.needs_tmpdir = 1,
};
