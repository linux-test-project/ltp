// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify that chmod(2) succeeds when used to change the mode permissions
 * of a file or directory.
 */

#include "tst_test.h"

#define MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"
#define TESTDIR		"testdir_1"

static int modes[] = {0, 07, 070, 0700, 0777, 02777, 04777, 06777};

static char *test_dir;
static char *test_file;

static struct variant {
	char **name;
	unsigned int mode_mask;
	char *desc;
} variants[] = {
	{&test_file, S_IFREG, "verify permissions of file"},
	{&test_dir, S_IFDIR, "verify permissions of directory"},
};

static void verify_chmod(unsigned int n)
{
	struct stat stat_buf;
	int mode = modes[n];
	struct variant *tc = &variants[tst_variant];

	TST_EXP_PASS(chmod(*tc->name, mode), "chmod(%s, %04o)",
	             *tc->name, mode);

	if (!TST_PASS)
		return;

	SAFE_STAT(*tc->name, &stat_buf);
	stat_buf.st_mode &= ~tc->mode_mask;

	if (stat_buf.st_mode == (unsigned int)mode) {
		tst_res(TPASS, "stat(%s) mode=%04o",
				*tc->name, stat_buf.st_mode);
	} else {
		tst_res(TFAIL, "stat(%s) mode=%04o",
				*tc->name, stat_buf.st_mode);
	}
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);

	if (tst_variant)
		SAFE_MKDIR(*variants[tst_variant].name, MODE);
	else
		SAFE_TOUCH(*variants[tst_variant].name, MODE, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test_variants = ARRAY_SIZE(variants),
	.tcnt = ARRAY_SIZE(modes),
	.test = verify_chmod,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&test_file, .str = TESTFILE},
		{&test_dir, .str = TESTDIR},
		{}
	}
};
