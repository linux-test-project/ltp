// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2013 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2003-2023
 * Ported to LTP: Wayne Boyer
 * 11/2013 Ported by Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 * 11/2016 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * -  access() fails with -1 return value and sets errno to EINVAL
 *    if the specified access mode argument is invalid.
 * -  access() fails with -1 return value and sets errno to ENOENT
 *    if the specified file doesn't exist (or pathname is NULL).
 * -  access() fails with -1 return value and sets errno to ENAMETOOLONG
 *    if the pathname size is > PATH_MAX characters.
 * -  access() fails with -1 return value and sets errno to ENOTDIR
 *    if a component used as a directory in pathname is not a directory.
 * -  access() fails with -1 return value and sets errno to ELOOP
 *    if too many symbolic links were encountered in resolving pathname.
 * -  access() fails with -1 return value and sets errno to EROFS
 *    if write permission was requested for files on a read-only file system.
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
#define LONGPATHSIZE (PATH_MAX + 2)

static uid_t uid;
static char *longpathname;
static char *fname1;
static char *fname2;
static char *sname1;
static char *empty_fname;
static char *mnt_point;

static struct tcase {
	char **pathname;
	int mode;
	int exp_errno;
} tcases[] = {
	{&fname1, -1, EINVAL},
	{&empty_fname, W_OK, ENOENT},
	{&longpathname, R_OK, ENAMETOOLONG},
	{&fname2, R_OK, ENOTDIR},
	{&sname1, R_OK, ELOOP},
	{&mnt_point, W_OK, EROFS}
};

static void access_test(struct tcase *tc, const char *user)
{
	TST_EXP_FAIL(access(*tc->pathname, tc->mode), tc->exp_errno,
	             "access as %s", user);
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

	memset(longpathname, 'a', LONGPATHSIZE - 1);
	longpathname[LONGPATHSIZE-1] = 0;

	SAFE_TOUCH(FNAME1, 0333, NULL);
	SAFE_TOUCH(DNAME, 0644, NULL);

	SAFE_SYMLINK(SNAME1, SNAME2);
	SAFE_SYMLINK(SNAME2, SNAME1);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.forks_child = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
	.setup = setup,
	.test = verify_access,
	.bufs = (struct tst_buffers []) {
		{&fname1, .str = FNAME1},
		{&fname2, .str = FNAME2},
		{&sname1, .str = SNAME1},
		{&empty_fname, .str = ""},
		{&longpathname, .size = LONGPATHSIZE},
		{&mnt_point, .str = MNT_POINT},
		{}
	}
};
