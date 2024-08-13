// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies LANDLOCK_ACCESS_FS_IOCTL_DEV access in the
 * landlock sandbox by creating a pipe and testing that ioctl() can be executed
 * on it. The test is also verifying that some of the I/O operations can be
 * always executed no matter the sandbox rules.
 */

#include "landlock_common.h"
#include <sys/ioctl.h>

#define MNTPOINT "sandbox"
#define FILENAME MNTPOINT"/fifo"

static struct landlock_ruleset_attr *ruleset_attr;
static struct landlock_path_beneath_attr *path_beneath_attr;
static int file_fd = -1;
static int dev_fd = -1;

static void run(void)
{
	if (SAFE_FORK())
		return;

	int flag = 0;
	size_t sz = 0;

	TST_EXP_PASS(ioctl(file_fd, FIONREAD, &sz));
	TST_EXP_PASS(ioctl(dev_fd, FIOCLEX));
	TST_EXP_PASS(ioctl(dev_fd, FIONCLEX));
	TST_EXP_PASS(ioctl(dev_fd, FIONBIO, &flag));
	TST_EXP_PASS(ioctl(dev_fd, FIOASYNC, &flag));

	_exit(0);
}

static void setup(void)
{
	int ruleset_fd;

	if (verify_landlock_is_enabled() < 5)
		tst_brk(TCONF, "LANDLOCK_ACCESS_FS_IOCTL_DEV is not supported");

	SAFE_TOUCH(FILENAME, 0640, NULL);

	file_fd = SAFE_OPEN(FILENAME, O_RDONLY | O_NONBLOCK, 0640);
	dev_fd = SAFE_OPEN("/dev/zero", O_RDONLY | O_NONBLOCK, 0640);

	tst_res(TINFO, "Applying LANDLOCK_ACCESS_FS_IOCTL_DEV");

	ruleset_attr->handled_access_fs = LANDLOCK_ACCESS_FS_IOCTL_DEV;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0);

	apply_landlock_layer(
		ruleset_attr,
		path_beneath_attr,
		MNTPOINT,
		LANDLOCK_ACCESS_FS_IOCTL_DEV
	);

	SAFE_CLOSE(ruleset_fd);
}

static void cleanup(void)
{
	if (dev_fd != -1)
		SAFE_CLOSE(dev_fd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{&path_beneath_attr, .size = sizeof(struct landlock_path_beneath_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *[]) {
		"vfat",
		NULL
	},
};
