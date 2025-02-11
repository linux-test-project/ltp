// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *               Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 * Copyright (c) Linux Test Project, 2005-2023
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Check for basic errors returned by mount(2) system call.
 *
 * - ENODEV if filesystem type not configured
 * - ENOTBLK if specialfile is not a block device
 * - EBUSY if specialfile is already mounted or it  cannot  be remounted
 *   read-only, because it still holds files open for writing.
 * - EINVAL if specialfile or device is invalid or a remount was attempted,
 *   while source was not already mounted on target.
 * - EFAULT if special file or device file points to invalid address space.
 * - ENAMETOOLONG if pathname was longer than MAXPATHLEN.
 * - ENOENT if pathname was empty or has a nonexistent component.
 * - ENOTDIR if not a directory.
 */

#include "tst_test.h"
#include <sys/mount.h>

#define MNTPOINT "mntpoint"
#define TEST_FILE MNTPOINT"/file"

static char path[PATH_MAX + 2];
static const char *long_path = path;
static const char *device;
static const char *fs_type;
static const char *null;
static const char *wrong_fs_type = "error";
static const char *mntpoint = MNTPOINT;
static const char *fault;
static const char *nonexistent = "nonexistent";
static const char *char_dev = "char_device";
static const char *file = "filename";
static int fd;

static void pre_mount(void);
static void post_umount(void);
static void pre_create_file(void);
static void post_delete_file(void);

static struct test_case {
	const char **device;
	const char **mntpoint;
	const char **fs_type;
	const char *desc;
	unsigned long flag;
	int exp_errno;
	void (*setup)(void);
	void (*cleanup)(void);
} test_cases[] = {
	{.fs_type = &wrong_fs_type, .desc = "wrong FS type", .exp_errno = ENODEV},
	{.device = &char_dev, .desc = "char device", .exp_errno = ENOTBLK},
	{.desc = "mounted folder", .exp_errno = EBUSY, .setup = pre_mount, .cleanup = post_umount},
	{.desc = "mounted folder containing file", .flag = MS_REMOUNT | MS_RDONLY,
		.exp_errno = EBUSY, .setup = pre_create_file, .cleanup = post_delete_file},
	{.device = &null, .desc = "invalid device", .exp_errno = EINVAL},
	{.fs_type = &null, .desc = "invalid device type", .exp_errno = EINVAL},
	{.desc = "mounted folder", .flag = MS_REMOUNT, .exp_errno = EINVAL},
	{.device = &fault, .desc = "fault device", .exp_errno = EFAULT},
	{.fs_type = &fault, .desc = "fault device type", .exp_errno = EFAULT},
	{.mntpoint = &long_path, .desc = "long name", .exp_errno = ENAMETOOLONG},
	{.mntpoint = &nonexistent, .desc = "non existant folder", .exp_errno = ENOENT},
	{.device = &device, .mntpoint = &file, .desc = "file", .exp_errno = ENOTDIR},
};

static void pre_mount(void)
{
	SAFE_MOUNT(device, mntpoint, fs_type, 0, NULL);
}

static void post_umount(void)
{
	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static void pre_create_file(void)
{
	pre_mount();
	fd = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR, 0700);
}

static void post_delete_file(void)
{
	SAFE_CLOSE(fd);
	post_umount();
}

static void setup(void)
{
	fault = tst_get_bad_addr(NULL);

	device = tst_device->dev;
	fs_type = tst_device->fs_type;

	memset(path, 'a', PATH_MAX + 1);

	SAFE_MKNOD(char_dev, S_IFCHR | 0777, 0);
	SAFE_TOUCH(file, 0777, 0);
}

static void cleanup(void)
{
	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static void run(unsigned int i)
{
	struct test_case *tc = &test_cases[i];

	if (!tc->device)
		tc->device = &device;

	if (!tc->mntpoint)
		tc->mntpoint = &mntpoint;

	if (!tc->fs_type)
		tc->fs_type = &fs_type;

	if (tc->setup)
		tc->setup();

	TST_EXP_FAIL(mount(*tc->device, *tc->mntpoint, *tc->fs_type, tc->flag, NULL),
		tc->exp_errno,
		"mounting %s",
		tc->desc);

	if (tc->cleanup)
		tc->cleanup();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
};
