// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2014-2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test Name: chmod06
 *
 * Test Description:
 *   Verify that,
 *   1) chmod(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) chmod(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   3) chmod(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) chmod(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) chmod(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) chmod(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists.
 */
#include <pwd.h>
#include <errno.h>
#include "tst_test.h"

#define MODE_RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define TEST_FILE3	"t_file/tfile_3"
#define TEST_FILE4	"test_file4"
#define MNT_POINT	"mntpoint"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)

static char long_path[PATH_MAX + 2];

static uid_t nobody_uid;

static void set_root(void);
static void set_nobody(void);

static struct tcase {
	char *pathname;
	mode_t mode;
	int exp_errno;
	void (*setup)(void);
	void (*cleanup)(void);
} tc[] = {
	{TEST_FILE1, FILE_MODE, EPERM, set_nobody, set_root},
	{TEST_FILE2, FILE_MODE, EACCES, set_nobody, set_root},
	{(char *)-1, FILE_MODE, EFAULT, NULL, NULL},
	{NULL, FILE_MODE, EFAULT, NULL, NULL},
	{long_path, FILE_MODE, ENAMETOOLONG, NULL, NULL},
	{"", FILE_MODE, ENOENT, NULL, NULL},
	{TEST_FILE3, FILE_MODE, ENOTDIR, NULL, NULL},
	{MNT_POINT, FILE_MODE, EROFS, NULL, NULL},
	{TEST_FILE4, FILE_MODE, ELOOP, NULL, NULL},
};

static char *bad_addr;

void run(unsigned int i)
{
	if (tc[i].setup)
		tc[i].setup();

	TEST(chmod(tc[i].pathname, tc[i].mode));

	if (tc[i].cleanup)
		tc[i].cleanup();

	if (TST_RET != -1) {
		tst_res(TFAIL, "chmod succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc[i].exp_errno) {
		tst_res(TPASS | TTERRNO, "chmod failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "chmod failed unexpectedly; "
		        "expected %d - %s", tc[i].exp_errno,
			tst_strerrno(tc[i].exp_errno));
	}
}

void set_root(void)
{
	SAFE_SETEUID(0);
}

void set_nobody(void)
{
	SAFE_SETEUID(nobody_uid);
}

void setup(void)
{
	struct passwd *nobody;
	unsigned int i;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_uid = nobody->pw_uid;

	bad_addr = SAFE_MMAP(0, 1, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

	for (i = 0; i < ARRAY_SIZE(tc); i++) {
		if (!tc[i].pathname)
			tc[i].pathname = bad_addr;
	}

	SAFE_TOUCH(TEST_FILE1, 0666, NULL);
	SAFE_MKDIR(DIR_TEMP, MODE_RWX);
	SAFE_TOUCH(TEST_FILE2, 0666, NULL);
	SAFE_CHMOD(DIR_TEMP, FILE_MODE);
	SAFE_TOUCH("t_file", MODE_RWX, NULL);

	memset(long_path, 'a', PATH_MAX+1);

	/*
	 * create two symbolic links who point to each other for
	 * test ELOOP.
	 */
	SAFE_SYMLINK("test_file4", "test_file5");
	SAFE_SYMLINK("test_file5", "test_file4");
}

static void cleanup(void)
{
	SAFE_CHMOD(DIR_TEMP, MODE_RWX);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
