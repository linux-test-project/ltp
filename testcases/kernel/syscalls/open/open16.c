// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Wei Gao <wegao@suse.com>
 */

/*\
 * Verify restricted opening (:manpage:`open(2)` and :manpage:`openat(2)`) of
 * FIFOs and regular files in sticky directories. This protection is controlled via
 * ``/proc/sys/fs/protected_fifos`` and ``/proc/sys/fs/protected_regular`` sysctl values:
 *
 * - When set to "0", writing/opening is unrestricted.
 * - When set to "1", O_CREAT open on FIFOs/regular files we don't own in world-writable
 *   sticky directories is disallowed, unless they are owned by the owner of the directory.
 * - When set to "2", the same restriction also applies to group-writable sticky directories.
 *
 * This test requires root to modify ``/proc/sys/fs/protected_*`` sysctls and
 * to manage file ownership and permissions in sticky directories.
 */

#include <pwd.h>
#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_file_at.h"
#include "tst_uid.h"

#define DIR "ltp_tmp_check1"
#define TEST_FILE "test_file_1"
#define TEST_FIFO "test_fifo_1"
#define TEST_FIFO_PATH DIR "/" TEST_FIFO

static int dir_fd = -1;
static uid_t uid1, uid2;
static gid_t gid1;

static struct tcase {
	int exp_errno;
	uid_t owner_uid;
	int use_nobody_gid;
	mode_t dir_mode;
} tcases[] = {
	{
		.dir_mode = 0777 | S_ISVTX,
	},
	{
		.exp_errno = EACCES,
		.dir_mode = 0777 | S_ISVTX,
	},
	{
		.exp_errno = EACCES,
		.owner_uid = -1,
		.use_nobody_gid = 1,
		.dir_mode = 0030 | S_ISVTX,
	},
};

static void verify_open(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	SAFE_FILE_PRINTF(PATH_FS_PROTECTED_REGULAR, "%u", n);
	SAFE_FILE_PRINTF(PATH_FS_PROTECTED_FIFOS, "%u", n);

	if (tc->owner_uid != (uid_t)-1 || tc->use_nobody_gid) {
		gid_t gid = tc->use_nobody_gid ? gid1 : 0;

		SAFE_CHOWN(DIR, tc->owner_uid, gid);
	}

	if (tc->dir_mode)
		SAFE_CHMOD(DIR, tc->dir_mode);

	if (SAFE_FORK())
		return;

	SAFE_SETGID(gid1);
	SAFE_SETUID(uid2);

	TST_EXP_FD_OR_FAIL(openat(dir_fd, TEST_FILE, O_RDWR | O_CREAT, 0777),
			   tc->exp_errno, "openat %s (sysctl_val=%u)", TEST_FILE, n);
	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);

	TST_EXP_FD_OR_FAIL(open(TEST_FIFO_PATH, O_RDWR | O_CREAT, 0777),
			   tc->exp_errno, "open %s (sysctl_val=%u)", TEST_FIFO, n);
	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	uid1 = pw->pw_uid;
	gid1 = pw->pw_gid;
	uid2 = tst_get_free_uid(uid1);

	umask(0);
	SAFE_MKDIR(DIR, 0777 | S_ISVTX);
	dir_fd = SAFE_OPEN(DIR, O_DIRECTORY);

	int fd = SAFE_OPENAT(dir_fd, TEST_FILE, O_CREAT | O_RDWR, 0777);

	SAFE_CLOSE(fd);
	SAFE_MKFIFO(TEST_FIFO_PATH, 0777);
	SAFE_CHOWN(TEST_FIFO_PATH, uid1, gid1);
	SAFE_CHOWN(DIR "/" TEST_FILE, uid1, gid1);
}

static void cleanup(void)
{
	if (dir_fd != -1)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_open,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.save_restore = (const struct tst_path_val[]) {
		{PATH_FS_PROTECTED_REGULAR, NULL, TST_SR_TCONF},
		{PATH_FS_PROTECTED_FIFOS, NULL, TST_SR_TCONF},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "30aba6656f61"},
		{"CVE", "2000-1134"},
		{"CVE", "2007-3852"},
		{"CVE", "2008-0525"},
		{"CVE", "2009-0416"},
		{"CVE", "2011-4834"},
		{"CVE", "2015-1838"},
		{"CVE", "2015-7442"},
		{"CVE", "2016-7489"},
		{}
	}
};
