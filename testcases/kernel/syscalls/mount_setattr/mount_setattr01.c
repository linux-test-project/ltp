// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Dai Shili <daisl.fnst@fujitsu.com>
 * Author: Chen Hanxiao <chenhx.fnst@fujitsu.com>
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Basic mount_setattr()/open_tree_attr() test.
 * Test whether the basic mount attributes are set correctly.
 *
 * Verify some MOUNT_SETATTR(2) attributes:
 *
 * - MOUNT_ATTR_RDONLY - makes the mount read-only
 * - MOUNT_ATTR_NOSUID - causes the mount not to honor the
 *   set-user-ID and set-group-ID mode bits and file capabilities
 *   when executing programs.
 * - MOUNT_ATTR_NODEV - prevents access to devices on this mount
 * - MOUNT_ATTR_NOEXEC - prevents executing programs on this mount
 * - MOUNT_ATTR_NOSYMFOLLOW - prevents following symbolic links
 *   on this mount
 * - MOUNT_ATTR_NODIRATIME - prevents updating access time for
 *   directories on this mount
 *
 * The mount_setattr functionality was added in v5.12, while the open_tree_attr
 * functionality was added in v6.15.
 */

#define _GNU_SOURCE

#include <sys/statvfs.h>
#include "tst_test.h"
#include "lapi/fsmount.h"
#include "lapi/syscalls.h"

#define MNTPOINT        "mntpoint"
#define OT_MNTPOINT     "ot_mntpoint"
#define TCASE_ENTRY(attrs, exp_attrs)   \
	{                                \
		.name = #attrs,                 \
		.mount_attrs = attrs,           \
		.expect_attrs = exp_attrs       \
	}

static int open_tree_variant1(struct mount_attr *attr);
static int open_tree_variant2(struct mount_attr *attr);

static int mount_flag, otfd = -1;
struct mount_attr *attr;

static struct tsetattr_variant {
	int (*child_variant)(struct mount_attr *attr);
	char *desc;
} tvariants[] = {
#if (__NR_mount_setattr != __LTP__NR_INVALID_SYSCALL)
	{ .child_variant = &open_tree_variant1, "mount_setattr()" },
#endif
#if (__NR_open_tree_attr != __LTP__NR_INVALID_SYSCALL)
	{ .child_variant = &open_tree_variant2, "open_tree_attr()"},
#endif
};

static struct tcase {
	char *name;
	unsigned int mount_attrs;
	unsigned int expect_attrs;
} tcases[] = {
	TCASE_ENTRY(MOUNT_ATTR_RDONLY, ST_RDONLY),
	TCASE_ENTRY(MOUNT_ATTR_NOSUID, ST_NOSUID),
	TCASE_ENTRY(MOUNT_ATTR_NODEV, ST_NODEV),
	TCASE_ENTRY(MOUNT_ATTR_NOEXEC, ST_NOEXEC),
	TCASE_ENTRY(MOUNT_ATTR_NOSYMFOLLOW, ST_NOSYMFOLLOW),
	TCASE_ENTRY(MOUNT_ATTR_NODIRATIME, ST_NODIRATIME),
};

static void cleanup(void)
{
	if (otfd > -1)
		SAFE_CLOSE(otfd);
	if (mount_flag)
		SAFE_UMOUNT(OT_MNTPOINT);
}

static void setup(void)
{
	fsopen_supported_by_kernel();

	if (access(OT_MNTPOINT, F_OK) != 0)
		SAFE_MKDIR(OT_MNTPOINT, 0777);
}

static int open_tree_variant1(struct mount_attr *attr)
{
	tst_res(TINFO, "Using variant open_tree() + mount_setattr()");

	otfd = TST_EXP_FD_SILENT(open_tree(AT_FDCWD, MNTPOINT,
			AT_EMPTY_PATH | OPEN_TREE_CLONE));
	if (otfd == -1)
		return -1;

	TST_EXP_PASS(mount_setattr(otfd, "", AT_EMPTY_PATH,
			attr, sizeof(*attr)));
	if (TST_RET == -1) {
		SAFE_CLOSE(otfd);
		return -1;
	}

	return otfd;
}

static int open_tree_variant2(struct mount_attr *attr)
{
	otfd = TST_EXP_FD(open_tree_attr(AT_FDCWD, MNTPOINT,
			AT_EMPTY_PATH | OPEN_TREE_CLONE,
			attr, sizeof(*attr)));

	return otfd;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct tsetattr_variant *tv = &tvariants[tst_variant];
	struct statvfs buf;

	tst_res(TINFO, "Using variant %s", tv->desc);

	memset(attr, 0, sizeof(*attr));
	attr->attr_set = tc->mount_attrs;

	otfd = tv->child_variant(attr);
	if (otfd == -1)
		goto out2;

	TST_EXP_PASS_SILENT(move_mount(otfd, "", AT_FDCWD, OT_MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH));
	if (!TST_PASS)
		goto out1;

	mount_flag = 1;
	SAFE_CLOSE(otfd);

	TST_EXP_PASS_SILENT(statvfs(OT_MNTPOINT, &buf), "statvfs success");
	if (!TST_PASS)
		goto out2;

	if (buf.f_flag & tc->expect_attrs)
		tst_res(TPASS, "%s is actually set as expected", tc->name);
	else
		tst_res(TFAIL, "%s is not actually set as expected", tc->name);

	goto out2;

out1:
	SAFE_CLOSE(otfd);
out2:
	if (mount_flag)
		SAFE_UMOUNT(OT_MNTPOINT);

	mount_flag = 0;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = ARRAY_SIZE(tvariants),
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []) {"fuse", NULL},
	.bufs = (struct tst_buffers []) {
		{&attr, .size = sizeof(struct mount_attr)},
		{}
	}
};
