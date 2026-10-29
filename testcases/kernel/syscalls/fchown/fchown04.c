// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Verify that:
 *
 * 1. fchown() returns -1 and sets errno to EPERM if the effective user id
 *    of process does not match the owner of the file and the process is
 *    not super user.
 * 2. fchown() returns -1 and sets errno to EBADF if the file descriptor
 *    of the specified file is not valid.
 * 3. fchown() returns -1 and sets errno to EROFS if the named file resides
 *    on a read-only file system.
 */

#include <pwd.h>

#include "tst_test.h"
#include "compat_tst_16.h"
#include "tst_safe_macros.h"

#define MNT_POINT	"mntpoint"
#define TEST_FILE	"tfile_1"
#define MODE		0666
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)

static int fd1;
static int fd2 = -1;
static int fd3;

static struct test_case_t {
	int *fd;
	int exp_errno;
} tc[] = {
	{&fd1, EPERM},
	{&fd2, EBADF},
	{&fd3, EROFS},
};

static void setup(void)
{
	struct passwd *ltpuser;

	fd1 = SAFE_OPEN(TEST_FILE, O_RDWR | O_CREAT, MODE);
	fd3 = SAFE_OPEN(MNT_POINT, O_RDONLY);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void run(unsigned int i)
{
	uid_t uid;
	gid_t gid;

	UID16_CHECK((uid = geteuid()), "fchown");
	GID16_CHECK((gid = getegid()), "fchown");

	TST_EXP_FAIL(FCHOWN(*tc[i].fd, uid, gid), tc[i].exp_errno,
	             "fchown(%i, %i, %i)", *tc[i].fd, uid, gid);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd3 > 0)
		SAFE_CLOSE(fd3);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
};
