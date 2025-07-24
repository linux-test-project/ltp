// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Wei Gao <wegao@suse.com>
 */

/*\
 * This test is checking if the propagation field of the
 * mount_attr structure is handled properly.
 *
 * - EINVAL with propagation set to -1
 * - When propagation is set to 0 it's not changed
 * - MS_SHARED turns propagation on
 * - MS_SLAVE turns propagation off
 * - MS_PRIVATE turns propagation off
 */

#define _GNU_SOURCE

#include <sys/statvfs.h>
#include "tst_test.h"
#include "lapi/fsmount.h"
#include "tst_safe_stdio.h"

static char *tmpdir;
static char slavedir[PATH_MAX];
static int mounted;

enum mount_type {
	MOUNT_TYPE_SHARED,
	MOUNT_TYPE_MASTER
};

static bool check_mount_type(const char *path, enum mount_type type_to_check)
{
	FILE *file = SAFE_FOPEN("/proc/self/mountinfo", "r");

	char line[PATH_MAX];
	bool found = false;

	while (fgets(line, sizeof(line), file)) {
		char mntpoint[PATH_MAX];
		char opts[256];

		if (sscanf(line, "%*d %*d %*d:%*d %*s %255s %*s %255s",
					mntpoint, opts) != 2)
			continue;

		if (strcmp(mntpoint, path) != 0)
			continue;

		switch (type_to_check) {
		case MOUNT_TYPE_SHARED:
			if (strstr(opts, "shared:") != NULL)
				found = true;
			break;
		case MOUNT_TYPE_MASTER:
			if (strstr(opts, "master:") != NULL)
				found = true;
			break;
		default:
			tst_res(TFAIL, "Unexpected mount_type value: %d", type_to_check);
		}
	}

	fclose(file);
	return found;
}

static void cleanup(void)
{
	if (mounted)
		SAFE_UMOUNT(tmpdir);
}

static void setup(void)
{
	tmpdir = tst_tmpdir_path();
	sprintf(slavedir, "%s/slavedir", tmpdir);
	SAFE_UNSHARE(CLONE_NEWNS);
	SAFE_MOUNT(NULL, "/", NULL, MS_REC | MS_PRIVATE, 0);
	SAFE_MOUNT("ltp-mount_setattr", tmpdir, "tmpfs", MS_NOATIME | MS_NODEV, "");
	SAFE_MKDIR(slavedir, 0777);
	mounted = 1;
}

static void run(void)
{
	struct mount_attr attr = {
		.attr_set       = 0,
		.attr_clr       = 0,
	};

	TST_EXP_PASS_SILENT(mount_setattr(-1, tmpdir, 0, &attr, sizeof(attr)));
	TST_EXP_EQ_LI(check_mount_type(tmpdir, MOUNT_TYPE_SHARED), 0);

	attr.propagation = -1;
	TST_EXP_FAIL_SILENT(mount_setattr(-1, tmpdir, 0, &attr, sizeof(attr)), EINVAL);
	TST_EXP_EQ_LI(check_mount_type(tmpdir, MOUNT_TYPE_SHARED), 0);

	attr.propagation = MS_SHARED;
	TST_EXP_PASS_SILENT(mount_setattr(-1, tmpdir, 0, &attr, sizeof(attr)));
	TST_EXP_EQ_LI(check_mount_type(tmpdir, MOUNT_TYPE_SHARED), 1);

	attr.propagation = 0;
	TST_EXP_PASS_SILENT(mount_setattr(-1, tmpdir, 0, &attr, sizeof(attr)));
	TST_EXP_EQ_LI(check_mount_type(tmpdir, MOUNT_TYPE_SHARED), 1);

	attr.propagation = MS_SLAVE;
	SAFE_MOUNT(tmpdir, slavedir, "none", MS_BIND, NULL);
	TST_EXP_EQ_LI(check_mount_type(slavedir, MOUNT_TYPE_SHARED), 1);
	TST_EXP_PASS_SILENT(mount_setattr(-1, slavedir, 0, &attr, sizeof(attr)));
	TST_EXP_EQ_LI(check_mount_type(slavedir, MOUNT_TYPE_MASTER), 1);
	TST_EXP_EQ_LI(check_mount_type(slavedir, MOUNT_TYPE_SHARED), 0);
	SAFE_UMOUNT(slavedir);

	attr.propagation = MS_PRIVATE;
	TST_EXP_PASS_SILENT(mount_setattr(-1, tmpdir, 0, &attr, sizeof(attr)));
	TST_EXP_EQ_LI(check_mount_type(tmpdir, MOUNT_TYPE_SHARED), 0);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
