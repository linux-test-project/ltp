// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 *   06/2019 Ported to new library: Christian Amann <camann@suse.com>
 */
/*
 * This test verifies that:
 *
 *   1) lstat(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   2) lstat(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists or empty string.
 *   3) lstat(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) lstat(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) lstat(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) lstat(2) returns -1 and sets errno to ELOOP if the pathname has too
 *	many symbolic links encountered while traversing.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tst_test.h"

#define MODE_RWX	0777
#define MODE_RW0	0666
#define TEST_DIR	"test_dir"
#define TEST_FILE	"test_file"

#define TEST_ELOOP	"/test_eloop"
#define TEST_ENOENT	""
#define TEST_EACCES	TEST_DIR"/test_eacces"
#define TEST_ENOTDIR	TEST_FILE"/test_enotdir"

static char longpathname[PATH_MAX + 2];
static char elooppathname[sizeof(TEST_ELOOP) * 43];
static struct stat stat_buf;

static struct test_case_t {
	char *pathname;
	int exp_errno;
} test_cases[] = {
	{TEST_EACCES, EACCES},
	{TEST_ENOENT, ENOENT},
	{NULL, EFAULT},
	{longpathname, ENAMETOOLONG},
	{TEST_ENOTDIR, ENOTDIR},
	{elooppathname, ELOOP},
};

static void run(unsigned int n)
{
	struct test_case_t *tc = &test_cases[n];

	TEST(lstat(tc->pathname, &stat_buf));

	if (TST_RET != -1) {
		tst_res(TFAIL | TTERRNO, "lstat() returned %ld, expected -1",
			TST_RET);
		return;
	}

	if (tc->exp_errno == TST_ERR) {
		tst_res(TPASS | TTERRNO, "lstat() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"lstat() failed unexpectedly; expected: %s - got",
			tst_strerrno(tc->exp_errno));
	}
}

static void setup(void)
{
	int i;
	struct passwd *ltpuser;

	/* Drop privileges for EACCES test */
	if (geteuid() == 0) {
		ltpuser = SAFE_GETPWNAM("nobody");
		SAFE_SETEUID(ltpuser->pw_uid);
	}

	memset(longpathname, 'a', PATH_MAX+1);
	longpathname[PATH_MAX+1] = '\0';

	SAFE_MKDIR(TEST_DIR, MODE_RWX);
	SAFE_TOUCH(TEST_EACCES, MODE_RWX, NULL);
	SAFE_TOUCH(TEST_FILE, MODE_RWX, NULL);
	SAFE_CHMOD(TEST_DIR, MODE_RW0);

	SAFE_MKDIR("test_eloop", MODE_RWX);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");
	/*
	 * NOTE: The ELOOP test is written based on the fact that the
	 * consecutive symlinks limit in the kernel is hardwired to 40.
	 */
	elooppathname[0] = '.';
	for (i = 0; i < 43; i++)
		strcat(elooppathname, TEST_ELOOP);
}

static void cleanup(void)
{
	SAFE_CHMOD(TEST_DIR, MODE_RWX);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
