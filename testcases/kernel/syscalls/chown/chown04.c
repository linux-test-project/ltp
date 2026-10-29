// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Verify that:
 *
 * 1. Chown() returns -1 and sets errno to EPERM if the effective user id
 *    of process does not match the owner of the file and the process is not
 *    super user.
 * 2. Chown() returns -1 and sets errno to EACCES if search permission is
 *    denied on a component of the path prefix.
 * 3. Chown() returns -1 and sets errno to EFAULT if pathname points outside
 *    user's accessible address space.
 * 4. Chown() returns -1 and sets errno to ENAMETOOLONG if the pathname
 *    component is too long.
 * 5. Chown() returns -1 and sets errno to ENOENT if the specified file does
 *    not exists.
 * 6. Chown() returns -1 and sets errno to ENOTDIR if the directory component
 *    in pathname is not a directory.
 * 7. Chown() returns -1 and sets errno to ELOOP if too many symbolic links
 *    were encountered in resolving pathname.
 * 8. Chown() returns -1 and sets errno to EROFS if the named file resides on
 *    a read-only filesystem.
 */

#include <pwd.h>

#include "tst_test.h"
#include "compat_tst_16.h"
#include "tst_safe_macros.h"

#define MODE 0666
#define MODE_RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)

#define MNT_POINT	"mntpoint"
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define TEST_FILE3	"t_file/tfile_3"
#define TEST_FILE4	"test_eloop1"
#define TEST_FILE5	"mntpoint"

static char long_path[PATH_MAX + 2] = {[0 ... PATH_MAX + 1] = 'a'};

static struct test_case_t {
	char *pathname;
	int exp_errno;
	char *desc;
} tc[] = {
	{TEST_FILE1, EPERM, "without permissions"},
	{TEST_FILE2, EACCES, "without full permissions of the path prefix"},
	{(char *)-1, EFAULT, "with unaccessible pathname points"},
	{long_path, ENAMETOOLONG, "when pathname is too long"},
	{"", ENOENT, "when file does not exist"},
	{TEST_FILE3, ENOTDIR, "when the path prefix is not a directory"},
	{TEST_FILE4, ELOOP, "with too many symbolic links"},
	{TEST_FILE5, EROFS, "when the named file resides on a read-only filesystem"}
};

static void run(unsigned int i)
{
	uid_t uid;
	gid_t gid;

	UID16_CHECK((uid = geteuid()), "chown");
	GID16_CHECK((gid = getegid()), "chown");

	TST_EXP_FAIL(CHOWN(tc[i].pathname, uid, gid), tc[i].exp_errno,
		     "chown() %s", tc[i].desc);
}

static void setup(void)
{
	struct passwd *ltpuser;

	tc[2].pathname = tst_get_bad_addr(NULL);

	SAFE_SYMLINK("test_eloop1", "test_eloop2");
	SAFE_SYMLINK("test_eloop2", "test_eloop1");

	SAFE_SETEUID(0);
	SAFE_TOUCH("t_file", MODE_RWX, NULL);
	SAFE_TOUCH(TEST_FILE1, MODE, NULL);
	SAFE_MKDIR(DIR_TEMP, S_IRWXU);
	SAFE_TOUCH(TEST_FILE2, MODE, NULL);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
	.setup = setup,
};
