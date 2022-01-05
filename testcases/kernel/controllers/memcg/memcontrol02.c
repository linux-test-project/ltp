// SPDX-License-Identifier: GPL-2.0
/*\
 *
 * [Description]
 *
 * Conversion of second kself test in cgroup/test_memcontrol.c.
 *
 * Original description:
 * "This test creates a memory cgroup, allocates some anonymous memory
 * and some pagecache and check memory.current and some memory.stat
 * values."
 *
 * Note that the V1 rss and cache counters were renamed to anon and
 * file in V2. Besides error reporting, this test differs from the
 * kselftest in the following ways:
 *
 * . It supports V1.
 * . It writes instead of reads to fill the page cache. Because no
 *   pages were allocated on tmpfs.
 * . It runs on most filesystems available
 * . On EXFAT and extN we change the margin of error between all and file
 *   memory to 50%. Because these allocate non-page-cache memory during writes.
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"
#include "tst_cgroup.h"

#define TMPDIR "mntdir"
#define MB(x) (x << 20)

static size_t page_size;
static const struct tst_cgroup_group *cg_test;
static struct tst_cgroup_group *cg_child;
static int fd;
static int file_to_all_error = 10;

/*
 * Checks if two given values differ by less than err% of their
 * sum. An extra percent is added for every doubling of the page size
 * to compensate for wastage in page sized allocations.
 */
static inline int values_close(const ssize_t a,
			       const ssize_t b,
			       const ssize_t err)
{
	const ssize_t page_adjusted_err = ffs(page_size >> 13) + err;

	return 100 * labs(a - b) <= (a + b) * page_adjusted_err;
}

static void alloc_anon_50M_check(void)
{
	const ssize_t size = MB(50);
	char *buf, *ptr;
	ssize_t anon, current;
	const char *const anon_key_fmt =
		TST_CGROUP_VER_IS_V1(cg_test, "memory") ? "rss %zd" : "anon %zd";

	buf = SAFE_MALLOC(size);
	for (ptr = buf; ptr < buf + size; ptr += page_size)
		*ptr = 0;

	SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zd", &current);
	TST_EXP_EXPR(current >= size,
		     "(memory.current=%zd) >= (size=%zd)", current, size);

	SAFE_CGROUP_LINES_SCANF(cg_child, "memory.stat", anon_key_fmt, &anon);

	TST_EXP_EXPR(anon > 0, "(memory.stat.anon=%zd) > 0", anon);
	TST_EXP_EXPR(values_close(size, anon, 3),
		     "(size=%zd) ~= (memory.stat.anon=%zd)", size, anon);
	TST_EXP_EXPR(values_close(anon, current, 3),
		     "(memory.current=%zd) ~= (memory.stat.anon=%zd)",
		     current, anon);
}

static void alloc_pagecache(const int fd, size_t size)
{
	char buf[BUFSIZ];
	size_t i;

	for (i = 0; i < size; i += sizeof(buf))
		SAFE_WRITE(1, fd, buf, sizeof(buf));
}

static void alloc_pagecache_50M_check(void)
{
	const size_t size = MB(50);
	size_t current, file;
	const char *const file_key_fmt =
		TST_CGROUP_VER_IS_V1(cg_test, "memory") ? "cache %zd" : "file %zd";

	fd = SAFE_OPEN(TMPDIR"/tmpfile", O_RDWR | O_CREAT, 0600);

	SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zu", &current);
	tst_res(TINFO, "Created temp file: memory.current=%zu", current);

	alloc_pagecache(fd, size);

	SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zu", &current);
	TST_EXP_EXPR(current >= size,
			 "(memory.current=%zu) >= (size=%zu)", current, size);

	SAFE_CGROUP_LINES_SCANF(cg_child, "memory.stat", file_key_fmt, &file);
	TST_EXP_EXPR(file > 0, "(memory.stat.file=%zd) > 0", file);

	TST_EXP_EXPR(values_close(file, current, file_to_all_error),
			 "(memory.current=%zd) ~= (memory.stat.file=%zd)",
			 current, file);

	SAFE_CLOSE(fd);
}

static void test_memcg_current(unsigned int n)
{
	size_t current;

	cg_child = tst_cgroup_group_mk(cg_test, "child");
	SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zu", &current);
	TST_EXP_EXPR(current == 0, "(current=%zu) == 0", current);

	if (!SAFE_FORK()) {
		SAFE_CGROUP_PRINTF(cg_child, "cgroup.procs", "%d", getpid());

		SAFE_CGROUP_SCANF(cg_child, "memory.current", "%zu", &current);
		tst_res(TINFO, "Added proc to memcg: memory.current=%zu",
			current);

		if (!n)
			alloc_anon_50M_check();
		else
			alloc_pagecache_50M_check();
	} else {
		tst_reap_children();
		cg_child = tst_cgroup_group_rm(cg_child);
	}
}

static void setup(void)
{
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);

	tst_cgroup_require("memory", NULL);
	cg_test = tst_cgroup_get_test_group();

	switch (tst_fs_type(TMPDIR)) {
	case TST_VFAT_MAGIC:
	case TST_EXFAT_MAGIC:
	case TST_EXT234_MAGIC:
		file_to_all_error = 50;
		break;
	}
}

static void cleanup(void)
{
	if (cg_child)
		cg_child = tst_cgroup_group_rm(cg_child);

	tst_cgroup_cleanup();
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = 2,
	.test = test_memcg_current,
	.mount_device = 1,
	.dev_min_size = 256,
	.mntpoint = TMPDIR,
	.all_filesystems = 1,
	.forks_child = 1,
	.needs_root = 1,
};
