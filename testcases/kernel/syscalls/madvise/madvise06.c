// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Red Hat, Inc.
 */

/*
 * DESCRIPTION
 *
 *   Page fault occurs in spite that madvise(WILLNEED) system call is called
 *   to prefetch the page. This issue is reproduced by running a program
 *   which sequentially accesses to a shared memory and calls madvise(WILLNEED)
 *   to the next page on a page fault.
 *
 *   This bug is present in all RHEL7 versions. It looks like this was fixed in
 *   mainline kernel > v3.15 by the following patch:
 *
 *   commit 55231e5c898c5c03c14194001e349f40f59bd300
 *   Author: Johannes Weiner <hannes@cmpxchg.org>
 *   Date:   Thu May 22 11:54:17 2014 -0700
 *
 *       mm: madvise: fix MADV_WILLNEED on shmem swapouts
 */

#include <errno.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/sysinfo.h>
#include "tst_test.h"

#define CHUNK_SZ (400*1024*1024L)
#define CHUNK_PAGES (CHUNK_SZ / pg_sz)
#define PASS_THRESHOLD (CHUNK_SZ / 4)

#define MNT_NAME "memory"
#define GROUP_NAME "madvise06"

static const char drop_caches_fname[] = "/proc/sys/vm/drop_caches";
static int pg_sz;

static void check_path(const char *path)
{
	if (access(path, R_OK | W_OK))
		tst_brk(TCONF, "file needed: %s\n", path);
}

static void setup(void)
{
	struct sysinfo sys_buf_start;

	pg_sz = getpagesize();

	tst_res(TINFO, "dropping caches");
	sync();
	SAFE_FILE_PRINTF(drop_caches_fname, "3");

	sysinfo(&sys_buf_start);
	if (sys_buf_start.freeram < 2 * CHUNK_SZ) {
		tst_brk(TCONF, "System RAM is too small (%li bytes needed)",
			2 * CHUNK_SZ);
	}
	if (sys_buf_start.freeswap < 2 * CHUNK_SZ) {
		tst_brk(TCONF, "System swap is too small (%li bytes needed)",
			2 * CHUNK_SZ);
	}

	SAFE_MKDIR(MNT_NAME, 0700);
	if (mount("memory", MNT_NAME, "cgroup", 0, "memory") == -1) {
		if (errno == ENODEV || errno == ENOENT)
			tst_brk(TCONF, "memory cgroup needed");
	}
	SAFE_MKDIR(MNT_NAME"/"GROUP_NAME, 0700);

	check_path("/proc/self/oom_score_adj");
	check_path(MNT_NAME"/"GROUP_NAME"/memory.limit_in_bytes");
	check_path(MNT_NAME"/"GROUP_NAME"/memory.swappiness");
	check_path(MNT_NAME"/"GROUP_NAME"/tasks");

	SAFE_FILE_PRINTF("/proc/self/oom_score_adj", "%d", -1000);
	SAFE_FILE_PRINTF(MNT_NAME"/"GROUP_NAME"/memory.limit_in_bytes", "%ld\n",
		PASS_THRESHOLD);
	SAFE_FILE_PRINTF(MNT_NAME"/"GROUP_NAME"/memory.swappiness", "60");
	SAFE_FILE_PRINTF(MNT_NAME"/"GROUP_NAME"/tasks", "%d\n", getpid());
}

static void cleanup(void)
{
	if (!access(MNT_NAME"/tasks", F_OK)) {
		SAFE_FILE_PRINTF(MNT_NAME"/tasks", "%d\n", getpid());
		SAFE_RMDIR(MNT_NAME"/"GROUP_NAME);
		SAFE_UMOUNT(MNT_NAME);
	}
}

static void dirty_pages(char *ptr, long size)
{
	long i;
	long pages = size / pg_sz;

	for (i = 0; i < pages; i++)
		ptr[i * pg_sz] = 'x';
}

static int get_page_fault_num(void)
{
	int pg;

	SAFE_FILE_SCANF("/proc/self/stat",
			"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d",
			&pg);
	return pg;
}

static void test_advice_willneed(void)
{
	int loops = 50;
	char *target;
	long swapcached_start, swapcached;
	int page_fault_num_1, page_fault_num_2;

	target = SAFE_MMAP(NULL, CHUNK_SZ, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS,
			-1, 0);
	dirty_pages(target, CHUNK_SZ);

	SAFE_FILE_LINES_SCANF("/proc/meminfo", "SwapCached: %ld",
		&swapcached_start);
	tst_res(TINFO, "SwapCached (before madvise): %ld", swapcached_start);

	TEST(madvise(target, CHUNK_SZ, MADV_WILLNEED));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "madvise failed");

	do {
		loops--;
		usleep(100000);
		SAFE_FILE_LINES_SCANF("/proc/meminfo", "SwapCached: %ld",
			&swapcached);
	} while (swapcached < swapcached_start + PASS_THRESHOLD / 1024
		&& loops > 0);

	tst_res(TINFO, "SwapCached (after madvise): %ld", swapcached);
	if (swapcached > swapcached_start + PASS_THRESHOLD / 1024) {
		tst_res(TPASS, "Regression test pass");
		SAFE_MUNMAP(target, CHUNK_SZ);
		return;
	}

	/*
	 * We may have hit a bug or we just have slow I/O,
	 * try accessing first page.
	 */
	page_fault_num_1 = get_page_fault_num();
	tst_res(TINFO, "PageFault(madvice / no mem access): %d",
			page_fault_num_1);
	target[0] = 'a';
	page_fault_num_2 = get_page_fault_num();
	tst_res(TINFO, "PageFault(madvice / mem access): %d",
			page_fault_num_2);

	if (page_fault_num_1 != page_fault_num_2)
		tst_res(TFAIL, "Bug has been reproduced");
	else
		tst_res(TPASS, "Regression test pass");

	SAFE_MUNMAP(target, CHUNK_SZ);
}

static struct tst_test test = {
	.test_all = test_advice_willneed,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "3.10.0",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "55231e5c898c"},
		{}
	}
};
