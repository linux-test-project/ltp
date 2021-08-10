// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *	 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Testcase to test whether chroot(2) sets errno correctly.
 *
 * - to test whether chroot() is setting ENAMETOOLONG if the
 *   pathname is more than VFS_MAXNAMELEN.
 * - to test whether chroot() is setting ENOTDIR if the argument
 *   is not a directory.
 * - to test whether chroot() is setting ENOENT if the directory
 *   does not exist.
 * - attempt to chroot to a path pointing to an invalid address
 *   and expect EFAULT as errno.
 * - to test whether chroot() is setting ELOOP if the two
 *   symbolic directory who point to each other.
 */

#include <stdio.h>
#include "tst_test.h"

static char fname[255];
static char nonexistent_dir[100] = "testdir";
static char bad_dir[] = "abcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
static char symbolic_dir[] = "sym_dir1";

static struct tcase {
	char *dir;
	int error;
	char *desc;
} tcases[] = {
	{bad_dir, ENAMETOOLONG, "chroot(longer than VFS_MAXNAMELEN)"},
	{fname, ENOTDIR, "chroot(not a directory)"},
	{nonexistent_dir, ENOENT, "chroot(does not exists)"},
	{(char *)-1, EFAULT, "chroot(an invalid address)"},
	{symbolic_dir, ELOOP, "chroot(symlink loop)"}
};

static void verify_chroot(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(chroot(tc->dir), tc->error, "%s", tc->desc);
}

static void setup(void)
{
	unsigned int i;

	(void)sprintf(fname, "tfile_%d", getpid());
	SAFE_TOUCH(fname, 0666, NULL);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].error == EFAULT)
			tcases[3].dir = tst_get_bad_addr(NULL);
	}

	SAFE_SYMLINK("sym_dir1/", "sym_dir2");
	SAFE_SYMLINK("sym_dir2/", "sym_dir1");
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_chroot,
	.needs_tmpdir = 1,
};
