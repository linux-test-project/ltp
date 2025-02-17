// SPDX-License-Identifier: GPL-2.0-only
/*\
 *
 * [Description]
 *
 * Conversion of the forth kself test in cgroup/test_memcontrol.c.
 *
 * Original description:
 * "First, this test creates the following hierarchy:
 * A       memory.low = 50M,  memory.max = 200M
 * A/B     memory.low = 50M,  memory.current = 50M
 * A/B/C   memory.low = 75M,  memory.current = 50M
 * A/B/D   memory.low = 25M,  memory.current = 50M
 * A/B/E   memory.low = 500M, memory.current = 0
 * A/B/F   memory.low = 0,    memory.current = 50M
 *
 * Usages are pagecache
 * Then it creates A/G and creates a significant
 * memory pressure in it.
 *
 * A/B    memory.current ~= 50M
 * A/B/C  memory.current ~= 33M
 * A/B/D  memory.current ~= 17M
 * A/B/E  memory.current ~= 0
 *
 * After that it tries to allocate more than there is unprotected
 * memory in A available, and checks that memory.low protects
 * pagecache even in this case."
 *
 * The closest thing to memory.low on V1 is soft_limit_in_bytes which
 * uses a different mechanism and has different semantics. So we only
 * test on V2 like the selftest. We do test on more file systems, but
 * not tempfs becaue it can't evict the page cache without swap. Also
 * we avoid filesystems which allocate extra memory for buffer heads.
 *
 * The tolerances have been increased from the self tests.
 */

#define _GNU_SOURCE

#include <inttypes.h>

#include "memcontrol_common.h"

#define TMPDIR "mntdir"

static struct tst_cg_group *trunk_cg[3];
static struct tst_cg_group *leaf_cg[4];
static int fd = -1;

enum checkpoints {
	CHILD_IDLE
};

enum trunk_cg {
	A,
	B,
	G
};

enum leaf_cg {
	C,
	D,
	E,
	F
};

static void cleanup_sub_groups(void)
{
	size_t i;

	for (i = ARRAY_SIZE(leaf_cg); i > 0; i--) {
		if (!leaf_cg[i - 1])
			continue;

		leaf_cg[i - 1] = tst_cg_group_rm(leaf_cg[i - 1]);
	}

	for (i = ARRAY_SIZE(trunk_cg); i > 0; i--) {
		if (!trunk_cg[i - 1])
			continue;

		trunk_cg[i - 1] = tst_cg_group_rm(trunk_cg[i - 1]);
	}
}

static void alloc_anon_in_child(const struct tst_cg_group *const cg,
				const size_t size)
{
	const pid_t pid = SAFE_FORK();

	if (pid) {
		tst_reap_children();
		return;
	}

	SAFE_CG_PRINTF(cg, "cgroup.procs", "%d", getpid());

	tst_res(TINFO, "Child %d in %s: Allocating anon: %"PRIdPTR,
		getpid(), tst_cg_group_name(cg), size);
	alloc_anon(size);

	exit(0);
}

static void alloc_pagecache_in_child(const struct tst_cg_group *const cg,
				     const size_t size)
{
	const pid_t pid = SAFE_FORK();

	if (pid) {
		tst_reap_children();
		return;
	}

	SAFE_CG_PRINTF(cg, "cgroup.procs", "%d", getpid());

	tst_res(TINFO, "Child %d in %s: Allocating pagecache: %"PRIdPTR,
		getpid(), tst_cg_group_name(cg), size);
	alloc_pagecache(fd, size);

	exit(0);
}

static void test_memcg_low(void)
{
	long c[4];
	unsigned int i;

	fd = SAFE_OPEN(TMPDIR"/tmpfile", O_RDWR | O_CREAT, 0600);
	trunk_cg[A] = tst_cg_group_mk(tst_cg, "trunk_A");

	SAFE_CG_SCANF(trunk_cg[A], "memory.low", "%ld", c);
	if (c[0]) {
		tst_brk(TCONF,
			"memory.low already set to %ld on parent group", c[0]);
	}

	SAFE_CG_PRINT(trunk_cg[A], "cgroup.subtree_control", "+memory");

	SAFE_CG_PRINT(trunk_cg[A], "memory.max", "200M");
	SAFE_CG_PRINT(trunk_cg[A], "memory.swap.max", "0");

	trunk_cg[B] = tst_cg_group_mk(trunk_cg[A], "trunk_B");

	SAFE_CG_PRINT(trunk_cg[B], "cgroup.subtree_control", "+memory");

	trunk_cg[G] = tst_cg_group_mk(trunk_cg[A], "trunk_G");

	for (i = 0; i < ARRAY_SIZE(leaf_cg); i++) {
		leaf_cg[i] = tst_cg_group_mk(trunk_cg[B],
						 "leaf_%c", 'C' + i);

		if (i == E)
			continue;

		alloc_pagecache_in_child(leaf_cg[i], MB(50));
	}

	SAFE_CG_PRINT(trunk_cg[A], "memory.low", "50M");
	SAFE_CG_PRINT(trunk_cg[B], "memory.low", "50M");
	SAFE_CG_PRINT(leaf_cg[C], "memory.low", "75M");
	SAFE_CG_PRINT(leaf_cg[D], "memory.low", "25M");
	SAFE_CG_PRINT(leaf_cg[E], "memory.low", "500M");
	SAFE_CG_PRINT(leaf_cg[F], "memory.low", "0");

	alloc_anon_in_child(trunk_cg[G], MB(148));

	SAFE_CG_SCANF(trunk_cg[B], "memory.current", "%ld", c);
	TST_EXP_EXPR(values_close(c[0], MB(50), 5),
		     "(A/B memory.current=%ld) ~= %d", c[0], MB(50));

	for (i = 0; i < ARRAY_SIZE(leaf_cg); i++)
		SAFE_CG_SCANF(leaf_cg[i], "memory.current", "%ld", c + i);

	TST_EXP_EXPR(values_close(c[0], MB(33), 20),
		     "(A/B/C memory.current=%ld) ~= %d", c[C], MB(33));
	TST_EXP_EXPR(values_close(c[1], MB(17), 20),
		     "(A/B/D memory.current=%ld) ~= %d", c[D], MB(17));
	TST_EXP_EXPR(values_close(c[2], 0, 1),
		     "(A/B/E memory.current=%ld) ~= 0", c[E]);
	tst_res(TINFO, "A/B/F memory.current=%ld", c[F]);

	alloc_anon_in_child(trunk_cg[G], MB(166));

	for (i = 0; i < ARRAY_SIZE(trunk_cg); i++) {
		long low, oom;
		const char id = "ABG"[i];

		SAFE_CG_LINES_SCANF(trunk_cg[i], "memory.events",
				    "low %ld", &low);
		SAFE_CG_LINES_SCANF(trunk_cg[i], "memory.events",
				    "oom %ld", &oom);

		tst_res(TINFO, "%c: low events=%ld, oom events=%ld",
			id, low, oom);
	}

	for (i = 0; i < ARRAY_SIZE(leaf_cg); i++) {
		long low, oom;
		const char id = 'C' + i;

		SAFE_CG_LINES_SCANF(leaf_cg[i], "memory.events",
				    "low %ld", &low);
		SAFE_CG_LINES_SCANF(leaf_cg[i], "memory.events",
				    "oom %ld", &oom);

		TST_EXP_EXPR(oom == 0, "(%c oom events=%ld) == 0", id, oom);

		if (i < E) {
			TST_EXP_EXPR(low > 0,
				"(%c low events=%ld) > 0", id, low);
		} else if (i == E) {
			TST_EXP_EXPR(low == 0,
				"(%c low events=%ld) == 0", id, low);
		} else if (!tst_cg_memory_recursiveprot(leaf_cg[F])) {
			/* dont not check F when recursive_protection enabled */
			TST_EXP_EXPR(low == 0,
				"(%c low events=%ld) == 0", id, low);
		}
	}

	cleanup_sub_groups();
	SAFE_CLOSE(fd);
	SAFE_UNLINK(TMPDIR"/tmpfile");
}

static void cleanup(void)
{
	cleanup_sub_groups();
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = test_memcg_low,
	.mount_device = 1,
	.mntpoint = TMPDIR,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const[]){
		"exfat", "vfat", "fuse", "ntfs", "tmpfs", NULL
	},
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.needs_cgroup_ver = TST_CG_V2,
	.needs_cgroup_ctrls = (const char *const[]){ "memory", NULL },
	.tags = (const struct tst_tag[]) {
		{
			"known-fail",
			"Low events in F: https://bugzilla.suse.com/show_bug.cgi?id=1196298"
		},
		{}
	},
};
