// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007, Hitachi, Ltd
 * Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>
 * Copyright (c) Linux Test Project, 2014-2018
 * Ported to LTP: Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * Verify that, utimes(2) returns -1 and sets errno to
 *
 * 1. EACCES if times is NULL, the caller's effective user ID does not match
 * the owner of the file, the caller does not have write access to the file,
 * and the caller is not privileged
 * 2. ENOENT if filename does not exist
 * 3. EFAULT if filename is NULL
 * 4. EPERM if times is not NULL, the caller's effective UID does not match
 * the owner of the file, and the caller is not privileged
 * 5. EROFS if path resides on a read-only file system
 */

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define MNT_POINT "mntpoint"
#define TESTFILE1 "testfile1"
#define TESTFILE2 "testfile2"
#define TESTFILE3 "mntpoint/file"
#define FILE_MODE (S_IRWXU | S_IRGRP | S_IXGRP | \
					S_IROTH | S_IXOTH)
#define DIR_MODE (S_IRWXU | S_IRWXG | S_IRWXO)

static struct timeval a_tv[2] = { {0, 0}, {1000, 0} };
static struct timeval m_tv[2] = { {1000, 0}, {0, 0} };
static struct timeval tv[2] = { {1000, 0}, {2000, 0} };

static struct tcase {
	char *pathname;
	struct timeval *times;
	int exp_errno;
} tcases[] = {
	{ TESTFILE1, a_tv, 0 },
	{ TESTFILE1, m_tv, 0 },
	{ TESTFILE2, NULL, EACCES },
	{ "notexistfile", tv, ENOENT },
	{ NULL, tv, EFAULT },
	{ TESTFILE2, tv, EPERM },
	{ TESTFILE3, tv, EROFS },
};

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	SAFE_TOUCH(TESTFILE2, FILE_MODE, NULL);
	SAFE_SETEUID(ltpuser->pw_uid);
	SAFE_TOUCH(TESTFILE1, FILE_MODE, NULL);
}

static void utimes_verify(unsigned int i)
{
	struct stat st;
	struct timeval tmp_tv[2];
	struct tcase *tc = &tcases[i];

	if (tc->exp_errno == 0) {
		SAFE_STAT(tc->pathname, &st);

		tmp_tv[0].tv_sec = st.st_atime;
		tmp_tv[0].tv_usec = 0;
		tmp_tv[1].tv_sec = st.st_mtime;
		tmp_tv[1].tv_usec = 0;
	}

	TEST(utimes(tc->pathname, tc->times));

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "utimes() worked as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"utimes() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, tst_strerrno(tc->exp_errno));
	}

	if (TST_ERR == 0 && utimes(tc->pathname, tmp_tv) == -1)
		tst_brk(TBROK | TERRNO, "utimes() failed.");
}

static struct tst_test test = {
	.setup = setup,
	.test = utimes_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
