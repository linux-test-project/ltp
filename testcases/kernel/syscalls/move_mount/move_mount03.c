// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Christian Brauner <brauner@kernel.org>
 * Copyright (c) 2023 Wei Gao <wegao@suse.com>
 */

/*\
 * Test allow to mount beneath top mount feature added in kernel 6.5:
 * 6ac392815628 ("fs: allow to mount beneath top mount")
 *
 * Test based on:
 * https://github.com/brauner/move-mount-beneath
 *
 * See also:
 *
 * - https://lore.kernel.org/all/20230202-fs-move-mount-replace-v4-0-98f3d80d7eaa@kernel.org/
 * - https://lwn.net/Articles/930591/
 * - https://github.com/brauner/move-mount-beneath
 */

/*
 * Test create for following commit:
 * commit 6ac392815628f317fcfdca1a39df00b9cc4ebc8b
 * Author: Christian Brauner <brauner@kernel.org>
 * Date:   Wed May 3 13:18:42 2023 +0200
 *     fs: allow to mount beneath top mount
 *
 * Above commit has heavily commented but i found following commit
 * contain simple summary of this feature for easy understanding:
 *
 * commit c0a572d9d32fe1e95672f24e860776dba0750a38
 * Author: Linus Torvalds <torvalds@linux-foundation.org>
 *       TL;DR:
 *
 *         > mount -t ext4 /dev/sda /mnt
 *           |
 *           --/mnt    /dev/sda    ext4
 *
 *         > mount --beneath -t xfs /dev/sdb /mnt
 *           |
 *           --/mnt    /dev/sdb    xfs
 *             --/mnt  /dev/sda    ext4
 *
 *         > umount /mnt
 *           |
 *           --/mnt    /dev/sdb    xfs
 *
 * So base above scenario design following scenario for LTP check:
 *
 *         > mount -t tmpfs /DIRA
 *           |
 *           --/DIRA(create A file within DIRA)
 *
 *         > mount -t tmpfs /DIRB
 *           |
 *           --/DIRA(create B file within DIRB)
 *
 *         > move_mount --beneath /DIRA /DIRB
 *           |
 *           --/mnt    /DIRA /DIRB
 *             --/mnt  /DIRB
 *
 *         If you check content of /DIRB, you can see file B
 *
 *         > umount /DIRB
 *           |
 *           --/mnt    /DIRA /DIRB
 *         Check content of /DIRB, you can see file A exist since
 *         current /DIRB mount source is already become /DIRA
 */

#include <stdio.h>

#include "tst_test.h"
#include "lapi/fsmount.h"
#include "lapi/sched.h"

#define DIRA "LTP_DIR_A"
#define DIRB "LTP_DIR_B"

static int fda = -1, fdb = -1;

static void run(void)
{
	SAFE_MOUNT("none", DIRA, "tmpfs", 0, 0);
	SAFE_MOUNT("none", DIRB, "tmpfs", 0, 0);
	SAFE_TOUCH(DIRA "/A", 0, NULL);
	SAFE_TOUCH(DIRB "/B", 0, NULL);

	fda = open_tree(AT_FDCWD, DIRA, OPEN_TREE_CLOEXEC | OPEN_TREE_CLONE);
	if (fda == -1)
		tst_brk(TBROK | TERRNO, "open_tree() failed");

	fdb = SAFE_OPEN(DIRB, O_PATH | O_NOFOLLOW, 0666);
	TST_EXP_PASS(move_mount(fda, "", fdb, "",
				MOVE_MOUNT_BENEATH | MOVE_MOUNT_F_EMPTY_PATH |
				MOVE_MOUNT_T_EMPTY_PATH));
	SAFE_CLOSE(fda);
	SAFE_CLOSE(fdb);

	TST_EXP_PASS(access(DIRB "/B", F_OK));
	TST_EXP_PASS(access(DIRA "/A", F_OK));
	TST_EXP_FAIL(access(DIRB "/A", F_OK), ENOENT, "A should not exist");

	SAFE_UMOUNT(DIRB);
	TST_EXP_PASS(access(DIRB "/A", F_OK));

	SAFE_UMOUNT(DIRB);
	SAFE_UMOUNT(DIRA);
}

static void setup(void)
{
	SAFE_MKDIR(DIRA, 0777);
	SAFE_MKDIR(DIRB, 0777);
}

static void cleanup(void)
{
	if (fda >= 0)
		SAFE_CLOSE(fda);

	if (fdb >= 0)
		SAFE_CLOSE(fdb);

	if (tst_is_mounted_at_tmpdir(DIRA))
		SAFE_UMOUNT(DIRA);

	if (tst_is_mounted_at_tmpdir(DIRB))
		SAFE_UMOUNT(DIRB);

	if (tst_is_mounted_at_tmpdir(DIRB))
		SAFE_UMOUNT(DIRB);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.min_kver = "6.5",
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "6ac392815628f"},
		{}
	},
};
