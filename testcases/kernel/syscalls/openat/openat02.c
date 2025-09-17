// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd Xing Gu <gux.fnst@cn.fujitsu.com>
 * Copyright (C) 2025 SUSE LLC Wei Gao <wegao@suse.com>
 */

/*\
 *   This test case will verify following scenarios of openat.
 *
 * - openat() succeeds to open a file in append mode, when
 *   'flags' is set to O_APPEND.
 *
 * - openat() succeeds to enable the close-on-exec flag for a
 *   file descriptor, when 'flags' is set to O_CLOEXEC.
 *
 * - openat() succeeds to allow files whose sizes cannot be
 *   represented in an off_t but can be represented in an off_t
 *   to be opened, when 'flags' is set to O_LARGEFILE.
 *
 * - openat() succeeds to not update the file last access time
 *   (st_atime in the inode) when the file is read, when 'flags'
 *   is set to O_NOATIME.
 *
 * - openat() succeeds to open the file failed if the file is a
 *   symbolic link, when 'flags' is set to O_NOFOLLOW.
 *
 * - openat() succeeds to truncate the file to length 0 if the file
 *   already exists and is a regular file and the open mode allows
 *   writing, when 'flags' is set to O_TRUNC.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "lapi/mount.h"
#include "tst_test.h"

#define TEST_APP "openat02_child"
#define MOUNT_POINT "mntpoint"
#define TEST_FILE MOUNT_POINT"/test_file"
#define SFILE MOUNT_POINT"/sfile"
#define LARGE_FILE MOUNT_POINT"/large_file"
#define STR "abcdefg"

static int dir_fd, fd;

static void testfunc_append(void);
static void testfunc_cloexec(void);
static void testfunc_largefile(void);
static void testfunc_noatime(void);
static void testfunc_nofollow(void);
static void testfunc_trunc(void);

static struct test_case {
	void (*testfunc)(void);
} test_cases[] = {
	{
		.testfunc = testfunc_append,
	},
	{
		.testfunc = testfunc_cloexec,
	},
	{
		.testfunc = testfunc_largefile,
	},
	{
		.testfunc = testfunc_noatime,
	},
	{
		.testfunc = testfunc_nofollow,
	},
	{
		.testfunc = testfunc_trunc,
	},
};

static void testfunc_append(void)
{
	off_t file_offset;

	TST_EXP_FD(openat(AT_FDCWD, TEST_FILE, O_APPEND | O_RDWR, 0777));

	SAFE_WRITE(SAFE_WRITE_ALL, TST_RET, STR, sizeof(STR) - 1);

	file_offset = SAFE_LSEEK(TST_RET, 0, SEEK_CUR);

	if (file_offset > (off_t)(sizeof(STR) - 1))
		tst_res(TPASS, "test O_APPEND for openat success");
	else
		tst_res(TFAIL, "test O_APPEND for openat failed");

	SAFE_CLOSE(TST_RET);
}

static void testfunc_cloexec(void)
{
	pid_t pid;
	int status;
	char buf[20];

	TST_EXP_FD(openat(AT_FDCWD, TEST_FILE, O_CLOEXEC | O_RDWR, 0777));

	sprintf(buf, "%ld", TST_RET);

	pid = SAFE_FORK();

	if (pid < 0)
		tst_brk(TBROK | TERRNO, "fork() failed");

	if (pid == 0) {
		if (execlp(TEST_APP, TEST_APP, buf, NULL))
			exit(2);
	}

	SAFE_CLOSE(TST_RET);

	SAFE_WAIT(&status);

	if (WIFEXITED(status)) {
		switch ((int8_t)WEXITSTATUS(status)) {
		case 0:
			tst_res(TPASS, "test O_CLOEXEC for openat success");
		break;
		case 1:
			tst_res(TFAIL, "test O_CLOEXEC for openat failed");
		break;
		default:
			tst_brk(TBROK, "execlp() failed");
		}
	} else {
		tst_brk(TBROK, "openat2_exec exits with unexpected error");
	}
}

static void testfunc_largefile(void)
{
	int fd;
	off_t offset;

	fd = SAFE_OPEN(LARGE_FILE,
				O_LARGEFILE | O_RDWR | O_CREAT, 0777);

	offset = lseek(fd, 4.1*1024*1024*1024, SEEK_SET);
	if (offset == -1)
		tst_brk(TBROK | TERRNO, "lseek64 failed");

	SAFE_WRITE(SAFE_WRITE_ALL, fd, STR, sizeof(STR) - 1);

	SAFE_CLOSE(fd);

	TST_EXP_FD(openat(AT_FDCWD, LARGE_FILE, O_LARGEFILE | O_RDONLY, 0777));

	if (TST_RET == -1) {
		tst_res(TFAIL, "test O_LARGEFILE for openat failed");
	} else {
		tst_res(TPASS, "test O_LARGEFILE for openat success");
		SAFE_CLOSE(TST_RET);
	}
}

static void testfunc_noatime(void)
{
	struct stat file_stat, file_newstat;
	char buf;
	const char *flags[] = {"noatime", "relatime", NULL};
	int ret;

	char path[PATH_MAX];
	char *tmpdir;

	tmpdir = tst_tmpdir_path();
	snprintf(path, sizeof(path), "%s/%s", tmpdir, MOUNT_POINT);
	ret = tst_path_has_mnt_flags(path, flags);
	if (ret > 0) {
		tst_res(TCONF, "test O_NOATIME flag for openat needs "
			"filesystems which are mounted without "
			"noatime and relatime");
		return;
	}

	SAFE_STAT(TEST_FILE, &file_stat);

	sleep(1);

	TST_EXP_FD(openat(AT_FDCWD, TEST_FILE, O_NOATIME | O_RDONLY, 0777));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "openat failed");
		return;
	}

	SAFE_READ(1, TST_RET, &buf, 1);

	SAFE_CLOSE(TST_RET);

	SAFE_STAT(TEST_FILE, &file_newstat);

	if (file_stat.st_atime == file_newstat.st_atime)
		tst_res(TPASS, "test O_NOATIME for openat success");
	else
		tst_res(TFAIL, "test O_NOATIME for openat failed");
}

static void testfunc_nofollow(void)
{
	TST_EXP_FD_OR_FAIL(openat(AT_FDCWD, SFILE, O_NOFOLLOW | O_RDONLY, 0777),
			   ELOOP);
}

static void testfunc_trunc(void)
{
	struct stat file_stat;

	TST_EXP_FD(openat(AT_FDCWD, TEST_FILE, O_TRUNC | O_RDWR, 0777));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "openat failed");
		return;
	}

	SAFE_FSTAT(TST_RET, &file_stat);

	if (file_stat.st_size == 0)
		tst_res(TPASS, "test O_TRUNC for openat success");
	else
		tst_res(TFAIL, "test O_TRUNC for openat failed");

	SAFE_CLOSE(TST_RET);
}

static void verify_openat(unsigned int n)
{
	struct test_case *tc = &test_cases[n];

	tc->testfunc();
}

static void setup(void)
{
	SAFE_FILE_PRINTF(TEST_FILE, "test file");
	SAFE_SYMLINK(TEST_FILE, SFILE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	if (dir_fd > 0)
		SAFE_CLOSE(dir_fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_openat,
	.tcnt = ARRAY_SIZE(test_cases),
	.forks_child = 1,
	.all_filesystems = 1,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_POINT,
	.filesystems = (struct tst_fs[]) {
		{
			.type = "ext2",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "ext3",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "ext4",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "xfs",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "btrfs",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "bcachefs",
			.mnt_flags = MS_STRICTATIME,
		},
		{
			.type = "tmpfs",
			.mnt_flags = MS_STRICTATIME,
		},
		{}
	},
	.skip_filesystems = (const char *[]) {
		"vfat",
		"exfat",
		"ntfs",
		NULL
	}
};
