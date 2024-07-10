// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: David Fenner, Jon Hendrickson
 * Copyright (C) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test verifies that chmod() is working correctly on symlink()
 * generated files.
 */

#include "tst_test.h"

#define PERMS 01777
#define TESTFILE "myobject"
#define SYMBNAME "my_symlink0"

static void run(void)
{
	struct stat oldsym_stat;
	struct stat newsym_stat;

	SAFE_TOUCH(TESTFILE, 0644, NULL);
	SAFE_SYMLINK(TESTFILE, SYMBNAME);
	SAFE_STAT(SYMBNAME, &oldsym_stat);

	TST_EXP_PASS(chmod(SYMBNAME, PERMS));
	SAFE_STAT(SYMBNAME, &newsym_stat);

	TST_EXP_EQ_LI(newsym_stat.st_mode & PERMS, PERMS);
	TST_EXP_EXPR(oldsym_stat.st_mode != newsym_stat.st_mode,
		"file mode has changed");

	SAFE_UNLINK(SYMBNAME);
	SAFE_UNLINK(TESTFILE);
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
