// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies LANDLOCK_ACCESS_FS_REFER access in the
 * landlock sandbox.
 *
 * [Algorithm]
 *
 * - apply LANDLOCK_ACCESS_FS_REFER in the folder1
 * - apply LANDLOCK_ACCESS_FS_REFER in the folder2
 * - create folder3
 * - verify that file can be moved from folder1 to folder2
 * - verify that file can't be moved from folder1 to folder3
 */

#include "landlock_common.h"

#define MNTPOINT "sandbox"
#define DIR1 MNTPOINT"/folder1"
#define DIR2 MNTPOINT"/folder2"
#define DIR3 MNTPOINT"/folder3"
#define FILENAME1 DIR1"/file"
#define FILENAME2 DIR2"/file"
#define FILENAME3 DIR3"/file"

static struct landlock_ruleset_attr *ruleset_attr;
static struct landlock_path_beneath_attr *path_beneath_attr;

static void run(void)
{
	if (SAFE_FORK())
		return;

	TST_EXP_PASS(rename(FILENAME1, FILENAME2));
	if (TST_RET == -1)
		return;

	TST_EXP_FAIL(rename(FILENAME2, FILENAME3), EXDEV);
	TST_EXP_PASS(rename(FILENAME2, FILENAME1));

	_exit(0);
}

static void setup(void)
{
	int abi;
	int ruleset_fd;

	abi = verify_landlock_is_enabled();
	if (abi < 2)
		tst_brk(TCONF, "LANDLOCK_ACCESS_FS_REFER is unsupported on ABI < 2");

	SAFE_MKDIR(DIR1, 0640);
	SAFE_MKDIR(DIR2, 0640);
	SAFE_MKDIR(DIR3, 0640);
	SAFE_TOUCH(FILENAME1, 0640, NULL);

	tst_res(TINFO, "Applying LANDLOCK_ACCESS_FS_REFER");

	ruleset_attr->handled_access_fs =
		LANDLOCK_ACCESS_FS_READ_FILE |
		LANDLOCK_ACCESS_FS_WRITE_FILE |
		LANDLOCK_ACCESS_FS_REFER;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0);

	apply_landlock_rule(
		path_beneath_attr,
		ruleset_fd,
		LANDLOCK_ACCESS_FS_REFER,
		DIR1);

	apply_landlock_rule(
		path_beneath_attr,
		ruleset_fd,
		LANDLOCK_ACCESS_FS_REFER,
		DIR2);

	enforce_ruleset(ruleset_fd);

	SAFE_CLOSE(ruleset_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
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
	.format_device = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *[]) {
		"vfat",
		"exfat",
		NULL
	},
};
