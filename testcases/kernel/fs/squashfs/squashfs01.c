// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
 */

/*\
 * Kernel commits
 *
 * - f37aa4c7366 (squashfs: add more sanity checks in id lookup)
 * - eabac19e40c (squashfs: add more sanity checks in inode lookup)
 * - 506220d2ba2 (squashfs: add more sanity checks in xattr id lookup)
 *
 * added some sanity checks, that verify the size of
 * inode lookup, id (uid/gid) and xattr blocks in the squashfs,
 * but broke mounting filesystems with completely filled blocks.
 * A block has a max size of 8192.
 * An inode lookup entry has an uncompressed size of 8 bytes,
 * an id block 4 bytes and an xattr block 16 bytes.
 *
 *
 * To fill up at least one block for each of the three tables,
 * 2048 files with unique uid/gid and xattr are created.
 *
 *
 * The bugs are fixed in kernel commits
 *
 * - c1b2028315c (squashfs: fix inode lookup sanity checks)
 * - 8b44ca2b634 (squashfs: fix xattr id and id lookup sanity checks)
 */

#include <stdio.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "tst_safe_macros.h"

static const char *MOUNT_DIR = "mnt";
static const char *DATA_DIR = "data";

static int mounted;

static void cleanup(void)
{
	if (mounted)
		SAFE_UMOUNT("mnt");
}

static void setup(void)
{
	int i;

	tst_res(TINFO, "Test squashfs sanity check regressions");

	SAFE_MKDIR(DATA_DIR, 0777);

	for (i = 0; i < 2048; ++i) {
		int fd;
		char name[20];

		sprintf(name, "%s/%d", DATA_DIR, i);
		fd = SAFE_OPEN(name, O_CREAT | O_EXCL, 0666);
		SAFE_FCHOWN(fd, i, i);

		/* This must be either "security", "user" or "trusted" namespace,
		 * because squashfs cannot store other namespaces.
		 * Since the files are most likely created on a tmpfs,
		 * "user" namespace is not possible, because it is not allowed.
		 */
		SAFE_FSETXATTR(fd, "security.x", &i, sizeof(i), 0);
		close(fd);
	}

	/* Create squashfs without any compression.
	 * This allows reasoning about block sizes.
	 * Redirect stdout, to get rid of undefined uid messages
	 */
	const char *argv[] = {
		"mksquashfs", DATA_DIR, tst_device->dev,
		"-noappend", "-noI", "-noD", "-noX", "-noF", NULL
	};
	tst_cmd(argv, "/dev/null", NULL, 0);

	SAFE_MKDIR(MOUNT_DIR, 0777);
}

static void run(void)
{
	if (mount(tst_device->dev, MOUNT_DIR, "squashfs", 0, NULL) != 0)
		tst_brk(TFAIL | TERRNO, "Mount failed");
	mounted = 1;

	SAFE_UMOUNT("mnt");
	mounted = 0;

	tst_res(TPASS, "Regression not detected");
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.setup = setup,
	.needs_root = 1,
	.needs_device = 1,
	.dev_min_size = 1,
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mksquashfs"},
		{}
	},
	.needs_drivers = (const char *const []) {
		"squashfs",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c1b2028315c"},
		{"linux-git", "8b44ca2b634"},
		{}
	},
};
