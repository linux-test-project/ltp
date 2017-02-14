/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Fujitsu Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Verify that,
 *  1) access() fails with -1 return value and sets errno to EINVAL
 *     if the specified access mode argument is invalid.
 *  2) access() fails with -1 return value and sets errno to ENOENT
 *     if the specified file doesn't exist (or pathname is NULL).
 *  3) access() fails with -1 return value and sets errno to ENAMETOOLONG
 *     if the pathname size is > PATH_MAX characters.
 *  4) access() fails with -1 return value and sets errno to ENOTDIR
 *     if a component used as a directory in pathname is not a directory.
 *  5) access() fails with -1 return value and sets errno to ELOOP
 *     if too many symbolic links were encountered in resolving pathname.
 *  6) access() fails with -1 return value and sets errno to EROFS
 *     if write permission was requested for files on a read-only file system.
 *
 *  07/2001 Ported by Wayne Boyer
 *  11/2013 Ported by Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *  11/2016 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>

#include "tst_test.h"

#define FNAME1	"accessfile1"
#define FNAME2	"accessfile2/accessfile2"
#define DNAME	"accessfile2"
#define SNAME1	"symlink1"
#define SNAME2	"symlink2"
#define MNT_POINT	"mntpoint"

static uid_t uid;
static char longpathname[PATH_MAX + 2];

static struct tcase {
	const char *pathname;
	int mode;
	int exp_errno;
} tcases[] = {
	{FNAME1, -1, EINVAL},
	{"", W_OK, ENOENT},
	{longpathname, R_OK, ENAMETOOLONG},
	{FNAME2, R_OK, ENOTDIR},
	{SNAME1, R_OK, ELOOP},
	{MNT_POINT, W_OK, EROFS}
};

static void access_test(struct tcase *tc, const char *user)
{
	TEST(access(tc->pathname, tc->mode));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "access as %s succeeded unexpectedly", user);
		return;
	}

	if (tc->exp_errno != TEST_ERRNO) {
		tst_res(TFAIL | TTERRNO,
			"access as %s should fail with %s",
			user, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "access as %s failed expectedly", user);
}

static void verify_access(unsigned int n)
{
	struct tcase *tc = tcases + n;
	pid_t pid;

	access_test(tc, "root");

	pid = SAFE_FORK();
	if (pid) {
		SAFE_WAITPID(pid, NULL, 0);
	} else {
		SAFE_SETUID(uid);
		access_test(tc, "nobody");
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	uid = pw->pw_uid;

	memset(longpathname, 'a', sizeof(longpathname) - 1);

	SAFE_TOUCH(FNAME1, 0333, NULL);
	SAFE_TOUCH(DNAME, 0644, NULL);

	SAFE_SYMLINK(SNAME1, SNAME2);
	SAFE_SYMLINK(SNAME2, SNAME1);
}

static struct tst_test test = {
	.tid = "access04",
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.mnt_flags = MS_RDONLY,
	.setup = setup,
	.test = verify_access,
};
