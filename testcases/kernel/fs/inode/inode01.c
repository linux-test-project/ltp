// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Exercise filesystem metadata and buffered I/O by building mixed trees of
 * directories and regular files, writing each file's pathname repeatedly,
 * then traversing the tree again to verify every object's type, size,
 * contents, link count and inode uniqueness.
 *
 * Two scenarios are run:
 *
 * - a single process building one small tree, and
 * - several workers building independent larger trees in parallel to add
 *   concurrent load, started simultaneously through a checkpoint.
 *
 * The test runs against every supported/mountable filesystem.
 */

#include "tst_test.h"

#define MAX_REPORTED_DUPS 10
#define MNTPOINT "mntpoint"

static struct tcase {
	unsigned int depth;
	unsigned int fanout;
	unsigned int repetitions;
	unsigned int workers;
	const char *desc;
} tcases[] = {
	{3, 4, 100, 1, "single-process small tree"},
	{6, 6, 8, 5, "parallel larger trees"},
};

struct inode_info {
	ino_t ino;
	char path[PATH_MAX];
};

static void create_tree(const struct tcase *tc, const char *parent,
			unsigned int level, unsigned int *id)
{
	unsigned int i;
	int create_file = level & 1;

	if (level >= tc->depth)
		return;

	for (i = 0; i < tc->fanout; i++, create_file = !create_file) {
		char path[PATH_MAX];

		snprintf(path, sizeof(path), "%s/%08u", parent, ++*id);
		if (create_file) {
			int fd = SAFE_OPEN(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
			unsigned int j;

			for (j = 0; j < tc->repetitions; j++)
				SAFE_WRITE(SAFE_WRITE_ALL, fd, path, strlen(path));
			SAFE_CLOSE(fd);
		} else {
			SAFE_MKDIR(path, 0777);
			create_tree(tc, path, level + 1, id);
		}
	}
}

static void record_inode(struct inode_info *inodes, unsigned int idx,
			 const char *path, ino_t ino)
{
	inodes[idx].ino = ino;
	snprintf(inodes[idx].path, sizeof(inodes[idx].path), "%s", path);
}

static ino_t verify_file(const struct tcase *tc, const char *path)
{
	char buf[PATH_MAX];
	char extra;
	struct stat st;
	size_t len = strlen(path);
	unsigned int i;
	int fd = SAFE_OPEN(path, O_RDONLY);

	SAFE_FSTAT(fd, &st);
	if (!S_ISREG(st.st_mode))
		tst_brk(TFAIL, "%s is not a regular file", path);

	/*
	 * No hardlinks are ever created, so every regular file must have
	 * exactly one link regardless of the underlying filesystem.
	 */
	if (st.st_nlink != 1) {
		tst_brk(TFAIL, "%s has st_nlink %lu, expected 1", path,
			(unsigned long)st.st_nlink);
	}

	if (st.st_size != (off_t)(len * tc->repetitions)) {
		tst_brk(TFAIL, "%s has size %lld, expected %zu", path,
			(long long)st.st_size, len * tc->repetitions);
	}

	for (i = 0; i < tc->repetitions; i++) {
		SAFE_READ(1, fd, buf, len);
		if (memcmp(buf, path, len))
			tst_brk(TFAIL, "%s contains unexpected data at record %u", path, i);
	}

	if (SAFE_READ(SAFE_READ_ANY, fd, &extra, 1))
		tst_brk(TFAIL, "%s has data after the expected records", path);

	SAFE_CLOSE(fd);

	return st.st_ino;
}

static void verify_tree(const struct tcase *tc, const char *parent,
			unsigned int level, unsigned int *id,
			struct inode_info *inodes)
{
	unsigned int i;
	int create_file = level & 1;

	if (level >= tc->depth)
		return;

	for (i = 0; i < tc->fanout; i++, create_file = !create_file) {
		char path[PATH_MAX];
		unsigned int my_id;
		ino_t ino;

		snprintf(path, sizeof(path), "%s/%08u", parent, ++*id);

		my_id = *id;

		if (create_file) {
			ino = verify_file(tc, path);
		} else {
			struct stat st;

			SAFE_STAT(path, &st);
			if (!S_ISDIR(st.st_mode))
				tst_brk(TFAIL, "%s is not a directory", path);

			/*
			 * Directory st_nlink is intentionally not checked:
			 * while many filesystems report 2 + subdirectory
			 * count, others (e.g. btrfs) always report 1 and
			 * never maintain a meaningful link count for
			 * directories, so asserting a specific value here
			 * would be a false failure on those filesystems.
			 */
			ino = st.st_ino;
			verify_tree(tc, path, level + 1, id, inodes);
		}

		record_inode(inodes, my_id - 1, path, ino);
	}
}

static int cmp_inode(const void *a, const void *b)
{
	const struct inode_info *ia = a;
	const struct inode_info *ib = b;

	if (ia->ino < ib->ino)
		return -1;

	if (ia->ino > ib->ino)
		return 1;

	return 0;
}

static unsigned int check_unique_inodes(struct inode_info *inodes,
					unsigned int total)
{
	unsigned int i;
	unsigned int dups = 0;

	qsort(inodes, total, sizeof(*inodes), cmp_inode);

	for (i = 1; i < total; i++) {
		if (inodes[i].ino != inodes[i - 1].ino)
			continue;

		if (++dups <= MAX_REPORTED_DUPS) {
			tst_res(TFAIL, "Duplicate inode %llu: %s and %s",
				(unsigned long long)inodes[i].ino,
				inodes[i - 1].path, inodes[i].path);
		}
	}

	if (dups > MAX_REPORTED_DUPS) {
		tst_res(TFAIL, "%u more duplicate inode(s) not shown",
			dups - MAX_REPORTED_DUPS);
	}

	return dups;
}

static void run_worker(const struct tcase *tc)
{
	char root[PATH_MAX];
	struct stat st;
	unsigned int id = 0;
	unsigned int total;
	struct inode_info *inodes;

	TST_CHECKPOINT_WAIT(0);

	snprintf(root, sizeof(root), MNTPOINT "/inode.%d", getpid());
	SAFE_MKDIR(root, 0777);
	create_tree(tc, root, 0, &id);
	total = id;

	SAFE_STAT(root, &st);
	if (!S_ISDIR(st.st_mode))
		tst_brk(TFAIL, "%s is not a directory", root);

	inodes = SAFE_MALLOC(total * sizeof(*inodes));

	id = 0;
	verify_tree(tc, root, 0, &id, inodes);

	if (!check_unique_inodes(inodes, total))
		tst_res(TPASS, "Created and verified %u objects in %s", total, root);

	free(inodes);
	tst_purge_dir(root);
	SAFE_RMDIR(root);
	exit(0);
}

static void run(unsigned int n)
{
	const struct tcase *tc = &tcases[n];
	unsigned int i;

	tst_res(TINFO, "Testing %s (depth=%u fanout=%u repetitions=%u workers=%u)",
		tc->desc, tc->depth, tc->fanout, tc->repetitions, tc->workers);

	for (i = 0; i < tc->workers; i++) {
		if (!SAFE_FORK())
			run_worker(tc);
	}

	TST_CHECKPOINT_WAKE2(0, tc->workers);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.needs_root = 1,
	.dev_min_size = 300,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.timeout = 300,
};
