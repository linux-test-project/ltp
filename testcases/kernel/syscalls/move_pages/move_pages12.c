// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) 2016-2019 FUJITSU LIMITED. All rights reserved.
 *  Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 *  Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *  Ported: Xiao Yang <yangx.jy@cn.fujitsu.com>
 *  Ported: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

/*\
 * [Description]
 *
 * *Test 1*
 *
 * This is a regression test for the race condition between move_pages()
 * and freeing hugepages, where move_pages() calls follow_page(FOLL_GET)
 * for hugepages internally and tries to get its refcount without
 * preventing concurrent freeing.
 *
 * This test can crash the buggy kernel, and the bug was fixed in:
 *
 *   commit e66f17ff71772b209eed39de35aaa99ba819c93d
 *   Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 *   Date:   Wed Feb 11 15:25:22 2015 -0800
 *
 *   mm/hugetlb: take page table lock in follow_huge_pmd()
 *
 * *Test 2.1*
 *
 * This is a regression test for the race condition, where move_pages()
 * and soft offline are called on a single hugetlb page concurrently.
 *
 * This test can crash the buggy kernel, and was fixed by:
 *
 *   commit c9d398fa237882ea07167e23bcfc5e6847066518
 *   Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 *   Date:   Fri Mar 31 15:11:55 2017 -0700
 *
 *   mm, hugetlb: use pte_present() instead of pmd_present() in follow_huge_pmd()
 *
 * *Test 2.2*
 *
 * This is also a regression test for an race condition causing SIGBUS
 * in hugepage migration/fault.
 *
 * This bug was fixed by:
 *
 *   commit 4643d67e8cb0b3536ef0ab5cddd1cedc73fa14ad
 *   Author: Mike Kravetz <mike.kravetz@oracle.com>
 *   Date:   Tue Aug 13 15:38:00 2019 -0700
 *
 *   hugetlbfs: fix hugetlb page migration/fault race causing SIGBUS
 *
 * *Test 2.3*
 *
 * The madvise() in the do_soft_online() was also triggering cases where soft
 * online returned EIO when page migration failed, which was fixed in:
 *
 *    commit 3f4b815a439adfb8f238335612c4b28bc10084d8
 *    Author: Oscar Salvador <osalvador@suse.de>
 *    Date:   Mon Dec 14 19:11:51 2020 -0800
 *
 *    mm,hwpoison: return -EBUSY when migration fails
 */

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "move_pages_support.h"
#include "lapi/mmap.h"

#ifdef HAVE_NUMA_V2

#define LOOPS	1000
#define PATH_MEMINFO	"/proc/meminfo"
#define PATH_NR_HUGEPAGES	"/proc/sys/vm/nr_hugepages"
#define PATH_HUGEPAGES	"/sys/kernel/mm/hugepages/"
#define TEST_NODES	2

static struct tcase {
	int tpages;
	int offline;
} tcases[] = {
	{2, 0},
	{2, 1},
};

static int pgsz, hpsz;
static long orig_hugepages = -1;
static char path_hugepages_node1[PATH_MAX];
static char path_hugepages_node2[PATH_MAX];
static long orig_hugepages_node1 = -1;
static long orig_hugepages_node2 = -1;
static unsigned int node1, node2;
static void *addr;

static int do_soft_offline(int tpgs)
{
	if (madvise(addr, tpgs * hpsz, MADV_SOFT_OFFLINE) == -1) {
		if (errno != EINVAL && errno != EBUSY)
			tst_res(TFAIL | TERRNO, "madvise failed");
		return errno;
	}
	return 0;
}

static void do_child(int tpgs)
{
	int test_pages = tpgs * hpsz / pgsz;
	int i, j;
	int *nodes, *status;
	void **pages;
	pid_t ppid = getppid();

	pages = SAFE_MALLOC(sizeof(char *) * test_pages);
	nodes = SAFE_MALLOC(sizeof(int) * test_pages);
	status = SAFE_MALLOC(sizeof(int) * test_pages);

	for (i = 0; i < test_pages; i++)
		pages[i] = addr + i * pgsz;

	for (i = 0; ; i++) {
		for (j = 0; j < test_pages; j++) {
			if (i % 2 == 0)
				nodes[j] = node1;
			else
				nodes[j] = node2;
			status[j] = 0;
		}

		TEST(numa_move_pages(ppid, test_pages,
			pages, nodes, status, MPOL_MF_MOVE_ALL));
		if (TST_RET < 0) {
			if (errno == ENOMEM)
				continue;

			tst_res(TFAIL | TTERRNO, "move_pages failed");
			break;
		}
	}

	exit(0);
}

static void do_test(unsigned int n)
{
	int i, ret;
	void *ptr;
	pid_t cpid = -1;
	int status;

	SAFE_FILE_PRINTF("/proc/sys/vm/compact_memory", "1");

	addr = SAFE_MMAP(NULL, tcases[n].tpages * hpsz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

	SAFE_MUNMAP(addr, tcases[n].tpages * hpsz);

	cpid = SAFE_FORK();
	if (cpid == 0)
		do_child(tcases[n].tpages);

	for (i = 0; i < LOOPS; i++) {
		ptr = mmap(NULL, tcases[n].tpages * hpsz,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
		if (ptr == MAP_FAILED) {
			if (i == 0)
				tst_brk(TBROK | TERRNO, "Cannot allocate hugepage");

			if (errno == ENOMEM) {
				usleep(1000);
				continue;
			}
		}

		if (ptr != addr)
			tst_brk(TBROK, "Failed to mmap at desired addr");

		memset(addr, 0, tcases[n].tpages * hpsz);

		if (tcases[n].offline) {
			ret = do_soft_offline(tcases[n].tpages);

			if (ret == EINVAL) {
				SAFE_KILL(cpid, SIGKILL);
				SAFE_WAITPID(cpid, &status, 0);
				SAFE_MUNMAP(addr, tcases[n].tpages * hpsz);
				tst_res(TCONF,
					"madvise() didn't support MADV_SOFT_OFFLINE");
				return;
			}
		}

		SAFE_MUNMAP(addr, tcases[n].tpages * hpsz);

		if (!tst_remaining_runtime())
			break;
	}

	SAFE_KILL(cpid, SIGKILL);
	SAFE_WAITPID(cpid, &status, 0);
	if (!WIFEXITED(status))
		tst_res(TPASS, "Bug not reproduced");
}

static void alloc_free_huge_on_node(unsigned int node, size_t size)
{
	char *mem;
	long ret;
	struct bitmask *bm;

	tst_res(TINFO, "Allocating and freeing %zu hugepages on node %u",
		size / hpsz, node);

	mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
	if (mem == MAP_FAILED) {
		if (errno == ENOMEM)
			tst_brk(TCONF, "Cannot allocate huge pages");

		tst_brk(TBROK | TERRNO, "mmap(..., MAP_HUGETLB, ...) failed");
	}

	bm = numa_bitmask_alloc(numa_max_possible_node() + 1);
	if (!bm)
		tst_brk(TBROK | TERRNO, "numa_bitmask_alloc() failed");

	numa_bitmask_setbit(bm, node);

	ret = mbind(mem, size, MPOL_BIND, bm->maskp, bm->size + 1, 0);
	if (ret) {
		if (errno == ENOMEM)
			tst_brk(TCONF, "Cannot mbind huge pages");

		tst_brk(TBROK | TERRNO, "mbind() failed");
	}

	TEST(mlock(mem, size));
	if (TST_RET) {
		SAFE_MUNMAP(mem, size);
		if (TST_ERR == ENOMEM || TST_ERR == EAGAIN)
			tst_brk(TCONF, "Cannot lock huge pages");
		tst_brk(TBROK | TTERRNO, "mlock failed");
	}

	numa_bitmask_free(bm);

	SAFE_MUNMAP(mem, size);
}

static void setup(void)
{
	int ret;
	long memfree;

	check_config(TEST_NODES);

	if (access(PATH_HUGEPAGES, F_OK))
		tst_brk(TCONF, "Huge page not supported");

	ret = get_allowed_nodes(NH_MEMS, TEST_NODES, &node1, &node2);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes: %d", ret);

	pgsz = (int)get_page_size();
	SAFE_FILE_LINES_SCANF(PATH_MEMINFO, "Hugepagesize: %d", &hpsz);

	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
	SAFE_FILE_LINES_SCANF(PATH_MEMINFO, "MemFree: %ld", &memfree);
	tst_res(TINFO, "Free RAM %ld kB", memfree);

	if (4 * hpsz > memfree)
		tst_brk(TBROK, "Not enough free RAM");

	snprintf(path_hugepages_node1, sizeof(path_hugepages_node1),
		 "/sys/devices/system/node/node%u/hugepages/hugepages-%dkB/nr_hugepages",
		 node1, hpsz);

	snprintf(path_hugepages_node2, sizeof(path_hugepages_node2),
		 "/sys/devices/system/node/node%u/hugepages/hugepages-%dkB/nr_hugepages",
		 node2, hpsz);

	if (!access(path_hugepages_node1, F_OK)) {
		SAFE_FILE_SCANF(path_hugepages_node1,
				"%ld", &orig_hugepages_node1);
		tst_res(TINFO,
			"Increasing %dkB hugepages pool on node %u to %ld",
			hpsz, node1, orig_hugepages_node1 + 4);
		SAFE_FILE_PRINTF(path_hugepages_node1,
				 "%ld", orig_hugepages_node1 + 4);
	}

	if (!access(path_hugepages_node2, F_OK)) {
		SAFE_FILE_SCANF(path_hugepages_node2,
				"%ld", &orig_hugepages_node2);
		tst_res(TINFO,
			"Increasing %dkB hugepages pool on node %u to %ld",
			hpsz, node2, orig_hugepages_node2 + 4);
		SAFE_FILE_PRINTF(path_hugepages_node2,
				 "%ld", orig_hugepages_node2 + 4);
	}

	hpsz *= 1024;

	if (orig_hugepages_node1 == -1 || orig_hugepages_node2 == -1) {
		SAFE_FILE_SCANF(PATH_NR_HUGEPAGES, "%ld", &orig_hugepages);
		tst_res(TINFO, "Increasing global hugepages pool to %ld",
			orig_hugepages + 8);
		SAFE_FILE_PRINTF(PATH_NR_HUGEPAGES, "%ld", orig_hugepages + 8);
	}

	alloc_free_huge_on_node(node1, 4L * hpsz);
	alloc_free_huge_on_node(node2, 4L * hpsz);
}

static void cleanup(void)
{
	if (orig_hugepages != -1)
		SAFE_FILE_PRINTF(PATH_NR_HUGEPAGES, "%ld", orig_hugepages);

	if (orig_hugepages_node1 != -1) {
		SAFE_FILE_PRINTF(path_hugepages_node1,
				 "%ld", orig_hugepages_node1);
	}

	if (orig_hugepages_node2 != -1) {
		SAFE_FILE_PRINTF(path_hugepages_node2,
				 "%ld", orig_hugepages_node2);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcases),
	.runtime = 240,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e66f17ff7177"},
		{"linux-git", "c9d398fa2378"},
		{"linux-git", "4643d67e8cb0"},
		{"linux-git", "3f4b815a439a"},
		{}
	}
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
