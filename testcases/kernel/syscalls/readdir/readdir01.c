// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

/*\
 * [Description]
 *
 * The test for the readdir(2) system call.
 * Create n files and check that readdir() finds n files
 */
#include <stdio.h>
#include "tst_test.h"

static const char prefix[] = "readdirfile";
static int nfiles = 10;

static void setup(void)
{
	char fname[255];
	int i;
	int fd;

	for (i = 0; i < nfiles; i++) {
		sprintf(fname, "%s_%d", prefix, i);
		fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "hello\n", 6);
		SAFE_CLOSE(fd);
	}
}

static void verify_readdir(void)
{
	int cnt = 0;
	DIR *test_dir;
	struct dirent *ent;

	test_dir = SAFE_OPENDIR(".");
	while ((ent = SAFE_READDIR(test_dir))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		if (!strncmp(ent->d_name, prefix, sizeof(prefix) - 1))
			cnt++;
	}

	if (cnt == nfiles) {
		tst_res(TPASS, "found all %d that were created", nfiles);
	} else {
		tst_res(TFAIL, "found %s files than were created, created: %d, found: %d",
					cnt > nfiles ? "more" : "less", nfiles, cnt);
	}

	SAFE_CLOSEDIR(test_dir);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_readdir,
	.needs_tmpdir = 1,
};
