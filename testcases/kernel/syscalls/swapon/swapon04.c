// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Check that :manpage:`swapon(2)` discard flags control swapon-time area discard:
 *
 * - With 0 (no flags), the swap area is not discarded at swapon.
 * - With SWAP_FLAG_DISCARD, the swap area is discarded at swapon.
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE, the swap area is discarded at swapon.
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_PAGES, swapon-time area discard
 *   is disabled (page-cluster discard only).
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | SWAP_FLAG_DISCARD_PAGES,
 *   SWAP_FLAG_DISCARD_ONCE takes precedence and discards the swap area at swapon.
 * - With SWAP_FLAG_DISCARD_ONCE or SWAP_FLAG_DISCARD_PAGES alone (without the
 *   SWAP_FLAG_DISCARD master enable bit), discard is not enabled.
 *
 * [Algorithm]
 *
 * - Create a backing file on the test filesystem and fill it completely.
 * - Attach the backing file to a loop device supporting discard.
 * - For each test case, populate the backing file and format it with mkswap.
 * - Record the allocated block count before swapon().
 * - Call swapon() with the test case flags.
 * - Verify whether the allocated block count dropped (discarded) or remained.
 * - Call swapoff() to reset the swap device state.
 */

#define _GNU_SOURCE

#include <sys/stat.h>
#include <sys/swap.h>
#include <unistd.h>

#include "tst_test.h"
#include "tse_swap.h"
#include "lapi/fallocate.h"

#define MNTPOINT "mntpoint"
#define BACKING_FILE MNTPOINT "/swap_backing_file"
#define SWAP_SIZE_MB 16

static char loop_dev[PATH_MAX];
static int loop_dev_id = -1;
static int loop_attached;
static int swap_active;
static size_t max_header_blocks;

static struct tcase {
	int flags;
	int exp_discard;
	const char *desc;
} tcases[] = {
	{
		.desc = "0 (no flags)",
	},
	{
		.flags = SWAP_FLAG_DISCARD,
		.exp_discard = 1,
		.desc = "SWAP_FLAG_DISCARD",
	},
	{
		.flags = SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE,
		.exp_discard = 1,
		.desc = "SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE",
	},
	{
		.flags = SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_PAGES,
		.desc = "SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_PAGES",
	},
	{
		.flags = SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | SWAP_FLAG_DISCARD_PAGES,
		.exp_discard = 1,
		.desc = "SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | SWAP_FLAG_DISCARD_PAGES",
	},
	{
		.flags = SWAP_FLAG_DISCARD_ONCE,
		.desc = "SWAP_FLAG_DISCARD_ONCE (alone without master flag)",
	},
	{
		.flags = SWAP_FLAG_DISCARD_PAGES,
		.desc = "SWAP_FLAG_DISCARD_PAGES (alone without master flag)",
	},
};

static void setup(void)
{
	char discard_path[PATH_MAX];
	unsigned long discard_max_bytes = 0;
	size_t page_size, blk_size, alloc_units;
	struct stat st;
	int fd;

	fd = SAFE_OPEN(BACKING_FILE, O_RDWR | O_CREAT | O_TRUNC, 0600);
	SAFE_FTRUNCATE(fd, SWAP_SIZE_MB * TST_MB);
	TEST(fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, 4096));
	SAFE_CLOSE(fd);

	if (TST_RET != 0) {
		if (TST_ERR == EOPNOTSUPP || TST_ERR == ENOSYS) {
			tst_brk(TCONF, "Filesystem %s does not support FALLOC_FL_PUNCH_HOLE",
				tst_device->fs_type);
		}

		tst_brk(TBROK | TERRNO, "fallocate() error");
	}

	loop_dev_id = tst_find_free_loopdev(loop_dev, sizeof(loop_dev));
	if (loop_dev_id < 0)
		tst_brk(TBROK, "No free loop device found");

	if (tst_attach_device(loop_dev, BACKING_FILE))
		tst_brk(TBROK, "Failed to attach %s to %s", loop_dev, BACKING_FILE);
	loop_attached = 1;

	snprintf(discard_path, sizeof(discard_path),
		 "/sys/block/loop%d/queue/discard_max_bytes", loop_dev_id);
	if (FILE_SCANF(discard_path, "%lu", &discard_max_bytes) != 0 || discard_max_bytes == 0) {
		tst_brk(TCONF, "Loop device %s does not support discard on %s",
			loop_dev, tst_device->fs_type);
	}

	SAFE_STAT(BACKING_FILE, &st);

	page_size = getpagesize();
	blk_size = st.st_blksize;
	alloc_units = page_size > blk_size ? page_size : blk_size;

	/* Minimum 512-byte blocks for 1 page + small tolerance for filesystem metadata */
	max_header_blocks = (alloc_units / 512) * 2;
}

static void verify_swapon(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd;
	struct stat st;
	blkcnt_t blocks_before, blocks_after;
	const char *const mkswap_argv[] = {"mkswap", loop_dev, NULL};

	tst_res(TINFO, "Testing swapon(%s, %s)", loop_dev, tc->desc);

	tst_fill_file(BACKING_FILE, 'A', TST_MB, SWAP_SIZE_MB);

	fd = SAFE_OPEN(BACKING_FILE, O_WRONLY);
	SAFE_FSYNC(fd);
	SAFE_CLOSE(fd);

	tst_cmd(mkswap_argv, "/dev/null", "/dev/null", 0);

	SAFE_STAT(BACKING_FILE, &st);
	blocks_before = st.st_blocks;

	if (!blocks_before)
		tst_brk(TBROK, "Backing file has 0 allocated blocks");

	TST_EXP_PASS(swapon(loop_dev, tc->flags), "swapon(%s, %s)", loop_dev, tc->desc);
	if (!TST_PASS)
		return;
	swap_active = 1;

	SAFE_STAT(BACKING_FILE, &st);
	blocks_after = st.st_blocks;

	if (tc->exp_discard)
		TST_EXP_LE_LU(blocks_after, max_header_blocks);
	else
		TST_EXP_EQ_LI(blocks_after, blocks_before);

	if (swapoff(loop_dev) != 0)
		tst_brk(TBROK | TERRNO, "swapoff(%s) failed", loop_dev);
	swap_active = 0;
}

static void cleanup(void)
{
	if (swap_active && swapoff(loop_dev) != 0)
		tst_res(TWARN | TERRNO, "swapoff(%s) failed", loop_dev);

	if (loop_attached)
		tst_detach_device(loop_dev);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []) {
		"vfat",
		"exfat",
		"ntfs",
		NULL
	},
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "mkswap"},
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SWAP=y",
		NULL
	},
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_swapon,
	.tcnt = ARRAY_SIZE(tcases),
};
