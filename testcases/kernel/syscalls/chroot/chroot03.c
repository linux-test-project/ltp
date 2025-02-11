// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *	 07/2001 Ported by Wayne Boyer
 */

/*\
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

#define FILE_NAME "test_file"
#define LOOP_DIR "sym_dir1"
#define NONEXISTENT_DIR "does_not_exist"

static char *longname_dir;
static char *file_name;
static char *nonexistent_dir;
static char *bad_ptr;
static char *loop_dir;

static struct tcase {
	char **dir;
	int error;
	char *desc;
} tcases[] = {
	{&longname_dir, ENAMETOOLONG, "chroot(longer than VFS_MAXNAMELEN)"},
	{&file_name, ENOTDIR, "chroot(not a directory)"},
	{&nonexistent_dir, ENOENT, "chroot(does not exists)"},
	{&bad_ptr, EFAULT, "chroot(an invalid address)"},
	{&loop_dir, ELOOP, "chroot(symlink loop)"}
};

static void verify_chroot(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(chroot(*tc->dir), tc->error, "%s", tc->desc);
}

static void setup(void)
{
	SAFE_TOUCH(FILE_NAME, 0666, NULL);
	bad_ptr = tst_get_bad_addr(NULL);

	memset(longname_dir, 'a', PATH_MAX + 1);
	longname_dir[PATH_MAX+1] = 0;

	SAFE_SYMLINK("sym_dir1/", "sym_dir2");
	SAFE_SYMLINK("sym_dir2/", "sym_dir1");
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_chroot,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&file_name, .str = FILE_NAME},
		{&nonexistent_dir, .str = NONEXISTENT_DIR},
		{&loop_dir, .str = LOOP_DIR},
		{&longname_dir, .size = PATH_MAX+2},
		{}
	}
};
