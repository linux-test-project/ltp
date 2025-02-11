// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2006-2024
 */

/*\
 * Check the basic functionality of the unlink(2):
 *
 * - unlink(2) can delete regular file successfully.
 * - unlink(2) can delete fifo file successfully.
 */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "tst_test.h"

static void file_create(char *name)
{
	sprintf(name, "tfile_%d", getpid());
	SAFE_TOUCH(name, 0777, NULL);
}

static void fifo_create(char *name)
{
	sprintf(name, "tfifo_%d", getpid());
	SAFE_MKFIFO(name, 0777);
}

static struct test_case_t {
	void (*setupfunc)(char *name);
	char *desc;
} tcases[] = {
	{file_create, "file"},
	{fifo_create, "fifo"},
};

static void verify_unlink(unsigned int n)
{
	char fname[255];
	struct test_case_t *tc = &tcases[n];

	tc->setupfunc(fname);

	TEST(unlink(fname));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "unlink(%s) failed", tc->desc);
		return;
	}

	if (!access(fname, F_OK)) {
		tst_res(TFAIL, "unlink(%s) succeeded, but %s still existed",
			tc->desc, tc->desc);
		return;
	}

	tst_res(TPASS, "unlink(%s) succeeded", tc->desc);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_unlink,
};
