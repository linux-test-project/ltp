// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   07/2001 Ported by Wayne Boyer
 *	 04/2003 Modified by Manoj Iyer - manjo@mail.utexas.edu
 */

/*\
 * [Description]
 *
 * Basic chroot() functionality test.
 *
 * - Create a file in the temporary directory
 * - Change the root to this temporary directory
 * - Check whether this file can be accessed in the new root directory
 */

#include <stdlib.h>
#include "tst_test.h"

#define TMP_FILENAME	"chroot02_testfile"
static char *path;

static void verify_chroot(void)
{
	struct stat buf;

	if (!SAFE_FORK()) {
		TST_EXP_PASS(chroot(path), "chroot(%s)", path);
		if (!TST_PASS)
			return;

		TST_EXP_PASS(stat("/" TMP_FILENAME, &buf), "stat(/%s)", TMP_FILENAME);
	}
}

static void setup(void)
{
	path = tst_get_tmpdir();
	SAFE_TOUCH(TMP_FILENAME, 0666, NULL);
}

static void cleanup(void)
{
	free(path);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.setup = setup,
	.test_all = verify_chroot,
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
};

