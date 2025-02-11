// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Linux Test Project, 2022
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * Check mount(2) system call with various flags.
 *
 * Verify that mount(2) syscall passes for each flag setting and validate
 * the flags:
 *
 * - MS_RDONLY - mount read-only
 * - MS_NODEV - disallow access to device special files
 * - MS_NOEXEC - disallow program execution
 * - MS_REMOUNT - alter flags of a mounted FS
 * - MS_NOSUID - ignore suid and sgid bits
 * - MS_NOATIME - do not update access times
 * - MS_NODIRATIME - only update access_time for directory instead of all types
 * - MS_STRICTATIME - always update access times
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <pwd.h>
#include "tst_test.h"
#include "lapi/mount.h"

#define MNTPOINT	"mntpoint"
#define TESTBIN	"mount03_suid_child"
#define BIN_PATH	MNTPOINT "/" TESTBIN
#define TEST_STR "abcdefghijklmnopqrstuvwxyz"
#define FILE_MODE	0644
#define SUID_MODE	(0511 | S_ISUID)

#define CHECK_ENOENT(x) ((x) == -1 && errno == ENOENT)

static int otfd;
static char file[PATH_MAX];
static char dir[PATH_MAX];
static uid_t nobody_uid;
static gid_t nobody_gid;

static void test_rdonly(void)
{
	snprintf(file, PATH_MAX, "%s/rdonly", MNTPOINT);
	TST_EXP_FAIL(otfd = open(file, O_CREAT | O_RDWR, 0700), EROFS);
}

static void test_nodev(void)
{
	snprintf(file, PATH_MAX, "%s/nodev", MNTPOINT);
	SAFE_MKNOD(file, S_IFBLK | 0777, 0);
	TST_EXP_FAIL(otfd = open(file, O_RDWR, 0700), EACCES);
	SAFE_UNLINK(file);
}

static void test_noexec(void)
{
	snprintf(file, PATH_MAX, "%s/noexec", MNTPOINT);
	otfd = SAFE_OPEN(file, O_CREAT | O_RDWR, 0700);
	TST_EXP_FAIL(execlp(file, basename(file), NULL), EACCES);
}

static void test_remount(void)
{
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, MS_REMOUNT, NULL);
	snprintf(file, PATH_MAX, "%s/remount", MNTPOINT);
	TST_EXP_FD(otfd = open(file, O_CREAT | O_RDWR, 0700));
}

static void test_nosuid(void)
{
	int ret;
	struct stat st;

	if (!SAFE_FORK()) {
		SAFE_CP(TESTBIN, BIN_PATH);

		ret = TST_RETRY_FN_EXP_BACKOFF(access(BIN_PATH, F_OK), !CHECK_ENOENT, 15);
		if (CHECK_ENOENT(ret))
			tst_brk(TBROK, "Timeout, %s does not exist", BIN_PATH);

		SAFE_STAT(BIN_PATH, &st);
		if (st.st_mode != SUID_MODE)
			SAFE_CHMOD(BIN_PATH, SUID_MODE);

		SAFE_SETREUID(nobody_uid, nobody_uid);
		SAFE_EXECL(BIN_PATH, BIN_PATH, NULL);
		tst_brk(TFAIL | TTERRNO, "Failed to execute %s", BIN_PATH);
	}

	tst_reap_children();
}

static void test_file_dir_noatime(int update_fatime, int update_datime)
{
	time_t atime, dir_atime;
	struct stat st, dir_st;
	char readbuf[20];
	DIR *test_dir;

	snprintf(file, PATH_MAX, "%s/noatime", MNTPOINT);
	TST_EXP_FD_SILENT(otfd = open(file, O_CREAT | O_RDWR, 0700));

	snprintf(dir, PATH_MAX, "%s/nodiratime", MNTPOINT);
	if (access(dir, F_OK) == -1 && errno == ENOENT)
		SAFE_MKDIR(dir, 0700);

	SAFE_WRITE(1, otfd, TEST_STR, strlen(TEST_STR));
	SAFE_FSTAT(otfd, &st);
	atime = st.st_atime;

	test_dir = SAFE_OPENDIR(dir);
	SAFE_STAT(dir, &dir_st);
	SAFE_READDIR(test_dir);
	SAFE_CLOSEDIR(test_dir);
	dir_atime = dir_st.st_atime;

	usleep(1001000);

	SAFE_READ(0, otfd, readbuf, sizeof(readbuf));
	SAFE_FSTAT(otfd, &st);

	test_dir = SAFE_OPENDIR(dir);
	SAFE_READDIR(test_dir);
	SAFE_CLOSEDIR(test_dir);
	SAFE_STAT(dir, &dir_st);

	if (update_fatime) {
		if (st.st_atime > atime)
			tst_res(TPASS, "st.st_atime(%ld) > atime(%ld)",
					st.st_atime, atime);
		else
			tst_res(TFAIL, "st.st_atime(%ld) < atime(%ld)",
					st.st_atime, atime);
	} else {
		TST_EXP_EQ_LI(st.st_atime, atime);
	}

	if (update_datime) {
		if (dir_st.st_atime > dir_atime)
			tst_res(TPASS, "dir_st.st_atime(%ld) > dir_atime(%ld)",
					dir_st.st_atime, dir_atime);
		else
			tst_res(TFAIL, "dir_st.st_atime(%ld) < dir_atime(%ld)",
					dir_st.st_atime, dir_atime);
	} else {
		TST_EXP_EQ_LI(dir_st.st_atime, dir_atime);
	}
}

static void test_noatime(void)
{
	test_file_dir_noatime(0, 0);
}

static void test_nodiratime(void)
{
	test_file_dir_noatime(1, 0);
}

static void test_strictatime(void)
{
	test_file_dir_noatime(1, 1);
}

#define FLAG_DESC(x) .flag = x, .flag2 = x, .desc = #x
#define FLAG_DESC2(x) .flag2 = x, .desc = #x
static struct tcase {
	unsigned int flag;
	unsigned int flag2;
	char *desc;
	void (*test)(void);
} tcases[] = {
	{FLAG_DESC(MS_RDONLY), test_rdonly},
	{FLAG_DESC(MS_NODEV), test_nodev},
	{FLAG_DESC(MS_NOEXEC), test_noexec},
	{MS_RDONLY, FLAG_DESC2(MS_REMOUNT), test_remount},
	{FLAG_DESC(MS_NOSUID), test_nosuid},
	{FLAG_DESC(MS_NOATIME), test_noatime},
	{FLAG_DESC(MS_NODIRATIME), test_nodiratime},
	{FLAG_DESC(MS_STRICTATIME), test_strictatime}
};

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	nobody_uid = ltpuser->pw_uid;
	nobody_gid = ltpuser->pw_gid;
}

static void cleanup(void)
{
	if (otfd >= 0)
		SAFE_CLOSE(otfd);

	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}


static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct statfs stfs;

	tst_res(TINFO, "Testing flag %s", tc->desc);

	TST_EXP_PASS_SILENT(mount(tst_device->dev, MNTPOINT, tst_device->fs_type,
		   tc->flag, NULL));
	if (!TST_PASS)
		return;

	if (tc->test)
		tc->test();

	SAFE_STATFS(MNTPOINT, &stfs);
	if (tc->flag == MS_STRICTATIME) {
		if (stfs.f_flags & (MS_NOATIME | MS_RELATIME))
			tst_res(TFAIL, "statfs() gets the incorrect mount flag");
		else
			tst_res(TPASS, "statfs() gets the correct mount flag");
		cleanup();
		return;
	}

	if (stfs.f_flags & tc->flag2)
		tst_res(TPASS, "statfs() gets the correct mount flag");
	else
		tst_res(TFAIL, "statfs() gets the incorrect mount flag");

	cleanup();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.format_device = 1,
	.resource_files = (const char *const[]) {
		TESTBIN,
		NULL,
	},
	.forks_child = 1,
	.child_needs_reinit = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){
		"exfat",
		"vfat",
		"ntfs",
		NULL
	},
};
