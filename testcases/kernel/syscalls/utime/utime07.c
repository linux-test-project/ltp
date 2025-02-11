// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: David Fenner, Jon Hendrickson
 * Copyright (C) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that utime() is working correctly on symlink()
 * generated files.
 *
 * Also verify if utime() fails with:
 *
 * - ENOENT when symlink points to nowhere
 * - ELOOP when symlink is looping
 */

#include <utime.h>
#include "tst_test.h"

#define TIME_DIFF 100

static void create_symlink(const char *path, const char *symname)
{
	struct stat asymlink;

	SAFE_SYMLINK(path, symname);
	SAFE_LSTAT(symname, &asymlink);

	if ((asymlink.st_mode & S_IFMT) != S_IFLNK) {
		tst_brk(TBROK, "symlink generated a non-symbolic link %s to %s",
			symname, path);
	}
}

static void test_utime(void)
{
	char *symname = "my_symlink0";
	struct stat oldsym_stat;
	struct stat newsym_stat;

	tst_res(TINFO, "Test if utime() changes access time");

	create_symlink(tst_tmpdir_path(), symname);
	SAFE_STAT(symname, &oldsym_stat);

	struct utimbuf utimes = {
		.actime = oldsym_stat.st_atime + TIME_DIFF,
		.modtime = oldsym_stat.st_mtime + TIME_DIFF
	};

	TST_EXP_PASS(utime(symname, &utimes));
	SAFE_STAT(symname, &newsym_stat);

	TST_EXP_EQ_LI(newsym_stat.st_atime - oldsym_stat.st_atime, TIME_DIFF);
	TST_EXP_EQ_LI(newsym_stat.st_mtime - oldsym_stat.st_mtime, TIME_DIFF);

	SAFE_UNLINK(symname);
}

static void test_utime_no_path(void)
{
	char *symname = "my_symlink1";
	struct utimbuf utimes;

	tst_res(TINFO, "Test if utime() raises ENOENT when symlink points to nowhere");

	create_symlink("bc+eFhi!k", symname);
	TST_EXP_FAIL(utime(symname, &utimes), ENOENT);

	SAFE_UNLINK(symname);
}

static void test_utime_loop(void)
{
	char *symname = "my_symlink2";
	struct utimbuf utimes;

	tst_res(TINFO, "Test if utime() raises ELOOP when symlink is looping");

	create_symlink(symname, symname);
	TST_EXP_FAIL(utime(symname, &utimes), ELOOP);

	SAFE_UNLINK(symname);
}

static void run(void)
{
	test_utime();
	test_utime_no_path();
	test_utime_loop();
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
