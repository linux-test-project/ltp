// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Testcase to test whether chdir(2) sets errno correctly.
 */

#include <errno.h>
#include "tst_test.h"

static char long_dir[] = "abcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
static char noexist_dir[] = "noexistdir";

static struct tcase {
	char *dir;
	int exp_errno;
} tcases[] = {
	{long_dir, ENAMETOOLONG},
	{noexist_dir, ENOENT},
	{0, EFAULT}, // bad_addr
};

static void verify_chdir(unsigned int i)
{
	TST_EXP_FAIL(chdir(tcases[i].dir), tcases[i].exp_errno, "chdir()");
}

static void setup(void)
{
	tcases[2].dir = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = verify_chdir,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
};
