// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Check that :manpage:`swapon(2)` discard flags control initial-discard
 * dispatch and its allocation effect:
 *
 * - With 0 (no flags), initial discard is not dispatched at swapon.
 * - With SWAP_FLAG_DISCARD, initial discard is dispatched at swapon.
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE, initial discard is
 *   dispatched at swapon.
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_PAGES, initial discard is
 *   disabled (page-cluster discard only).
 * - With SWAP_FLAG_DISCARD | SWAP_FLAG_DISCARD_ONCE | SWAP_FLAG_DISCARD_PAGES,
 *   SWAP_FLAG_DISCARD_ONCE takes precedence and dispatches initial discard.
 * - With SWAP_FLAG_DISCARD_ONCE or SWAP_FLAG_DISCARD_PAGES alone (without the
 *   SWAP_FLAG_DISCARD master enable bit), initial discard is not enabled.
 *
 * [Algorithm]
 *
 * - Create a backing file on the test filesystem and fill it completely.
 * - Attach the backing file to a loop device supporting discard.
 * - For each test case, populate the backing file and format it with mkswap.
 * - Record the allocated block count before swapon().
 * - Call swapon() with the test case flags.
 * - Verify the sectors reported discarded by the loop device and whether the
 *   allocated block count dropped or remained. The sector count checks initial
 *   discard dispatch and volume; allocation change is an effect indicator, not
 *   proof of complete backend reclamation.
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
#define LOOP_DISCARD_SECTORS_FORMAT \
	"%*s %*s %*s %*s %*s %*s %*s %*s " \
	"%*s %*s %*s %*s %*s %llu"

static char loop_dev[PATH_MAX];
static char loop_stat_path[PATH_MAX];
static char loop_iostats_path[PATH_MAX];
static int loop_dev_id = -1;
static int loop_attached;
static int swap_active;
static int orig_iostats;
static int iostats_restore;

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
	unsigned long long discard_sectors;
	unsigned long long swap_size = SWAP_SIZE_MB * TST_MB;
	long page_size;
	int iostats;
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

	snprintf(loop_stat_path, sizeof(loop_stat_path),
		 "/sys/block/loop%d/stat", loop_dev_id);
	snprintf(loop_iostats_path, sizeof(loop_iostats_path),
		 "/sys/block/loop%d/queue/iostats", loop_dev_id);
	if (FILE_SCANF(loop_stat_path, LOOP_DISCARD_SECTORS_FORMAT,
		       &discard_sectors) != 0) {
		tst_brk(TCONF, "Loop device %s does not expose discard sectors in %s",
			loop_dev, loop_stat_path);
	}

	if (FILE_SCANF(loop_iostats_path, "%d", &orig_iostats) != 0) {
		tst_brk(TCONF, "Loop device %s does not expose iostats in %s",
			loop_dev, loop_iostats_path);
	}
	if (!orig_iostats) {
		if (FILE_PRINTF(loop_iostats_path, "%d", 1) != 0)
			tst_brk(TCONF, "Could not enable iostats on %s", loop_dev);
		iostats_restore = 1;
		if (FILE_SCANF(loop_iostats_path, "%d", &iostats) != 0 || iostats != 1)
			tst_brk(TCONF, "Loop device %s does not support enabling iostats",
				loop_dev);
	}

	snprintf(discard_path, sizeof(discard_path),
		 "/sys/block/loop%d/queue/discard_max_bytes", loop_dev_id);
	if (FILE_SCANF(discard_path, "%lu", &discard_max_bytes) != 0 || discard_max_bytes == 0) {
		tst_brk(TCONF, "Loop device %s does not support discard on %s",
			loop_dev, tst_device->fs_type);
	}

	page_size = getpagesize();
	if (page_size <= 0 || swap_size % page_size || page_size % 512)
		tst_brk(TBROK, "Swap size %llu is not aligned to page size %ld and 512-byte sectors",
			swap_size, page_size);
}

static void verify_swapon(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd;
	struct stat st;
	blkcnt_t blocks_before, blocks_after;
	unsigned long long discard_sectors_before, discard_sectors_after;
	unsigned long long expected_discard_sectors = 0;
	const char *const mkswap_argv[] = {"mkswap", loop_dev, NULL};

	tst_res(TINFO, "Testing swapon(%s, %s)", loop_dev, tc->desc);

	if (tst_fill_file(BACKING_FILE, 'A', TST_MB, SWAP_SIZE_MB))
		tst_brk(TBROK | TERRNO, "fill %s failed", BACKING_FILE);

	fd = SAFE_OPEN(BACKING_FILE, O_WRONLY);
	SAFE_FSYNC(fd);
	SAFE_CLOSE(fd);

	tst_cmd(mkswap_argv, "/dev/null", "/dev/null", 0);

	SAFE_STAT(BACKING_FILE, &st);
	blocks_before = st.st_blocks;

	if (!blocks_before)
		tst_brk(TBROK, "Backing file has 0 allocated blocks");
	SAFE_FILE_SCANF(loop_stat_path, LOOP_DISCARD_SECTORS_FORMAT,
			&discard_sectors_before);
	if (tc->exp_discard)
		expected_discard_sectors =
			((unsigned long long)SWAP_SIZE_MB * TST_MB - getpagesize()) / 512;

	TST_EXP_PASS(swapon(loop_dev, tc->flags), "swapon(%s, %s)", loop_dev, tc->desc);
	if (!TST_PASS)
		return;
	swap_active = 1;

	SAFE_STAT(BACKING_FILE, &st);
	blocks_after = st.st_blocks;
	SAFE_FILE_SCANF(loop_stat_path, LOOP_DISCARD_SECTORS_FORMAT,
			&discard_sectors_after);

	TST_EXP_EQ_LU(discard_sectors_after - discard_sectors_before,
		      expected_discard_sectors);

	if (tc->exp_discard) {
		TST_EXP_EXPR(blocks_after < blocks_before,
			     "allocated blocks decreased: before=%llu, after=%llu",
			     (unsigned long long)blocks_before,
			     (unsigned long long)blocks_after);
	} else {
		TST_EXP_EQ_LI(blocks_after, blocks_before);
	}

	if (swapoff(loop_dev) != 0)
		tst_brk(TBROK | TERRNO, "swapoff(%s) failed", loop_dev);
	swap_active = 0;
}

static void cleanup(void)
{
	if (swap_active && swapoff(loop_dev) != 0)
		tst_res(TWARN | TERRNO, "swapoff(%s) failed", loop_dev);

	if (iostats_restore) {
		if (FILE_PRINTF(loop_iostats_path, "%d", orig_iostats) != 0)
			tst_res(TWARN, "Could not restore iostats on %s to %d",
				loop_dev, orig_iostats);
		iostats_restore = 0;
	}

	if (loop_attached) {
		tst_detach_device(loop_dev);
		loop_attached = 0;
	}
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
