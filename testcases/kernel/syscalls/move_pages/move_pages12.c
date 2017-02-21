/*
 * Copyright (c) 2016 Fujitsu Ltd.
 *  Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 *  Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a regression test for the race condition between move_pages()
 * and freeing hugepages, where move_pages() calls follow_page(FOLL_GET)
 * for hugepages internally and tries to get its refcount without
 * preventing concurrent freeing.
 *
 * This test can crash the buggy kernel, and the bug was fixed in:
 *
 *  commit e66f17ff71772b209eed39de35aaa99ba819c93d
 *  Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 *  Date:   Wed Feb 11 15:25:22 2015 -0800
 *
 *  mm/hugetlb: take page table lock in follow_huge_pmd()
 */

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "tst_test.h"
#include "move_pages_support.h"
#include "lapi/mmap.h"

#if HAVE_NUMA_MOVE_PAGES

#define LOOPS	1000
#define PATH_MEMINFO	"/proc/meminfo"
#define PATH_NR_HUGEPAGES	"/proc/sys/vm/nr_hugepages"
#define PATH_HUGEPAGES	"/sys/kernel/mm/hugepages/"
#define TEST_PAGES	2
#define TEST_NODES	2

static int pgsz, hpsz;
static long orig_hugepages;
static unsigned int node1, node2;
static void *addr;

static void do_child(void)
{
	int test_pages = TEST_PAGES * hpsz / pgsz;
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
		if (TEST_RETURN) {
			tst_res(TFAIL | TTERRNO, "move_pages failed");
			break;
		}
	}

	exit(0);
}

static void do_test(void)
{
	int i;
	pid_t cpid = -1;
	int status;

	addr = SAFE_MMAP(NULL, TEST_PAGES * hpsz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

	SAFE_MUNMAP(addr, TEST_PAGES * hpsz);

	cpid = SAFE_FORK();
	if (cpid == 0)
		do_child();

	for (i = 0; i < LOOPS; i++) {
		void *ptr;

		ptr = SAFE_MMAP(NULL, TEST_PAGES * hpsz,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
		if (ptr != addr)
			tst_brk(TBROK, "Failed to mmap at desired addr");

		memset(addr, 0, TEST_PAGES * hpsz);

		SAFE_MUNMAP(addr, TEST_PAGES * hpsz);
	}

	if (i == LOOPS) {
		SAFE_KILL(cpid, SIGKILL);
		SAFE_WAITPID(cpid, &status, 0);
		if (!WIFEXITED(status))
			tst_res(TPASS, "Bug not reproduced");
	}
}

static void setup(void)
{
	int memfree, ret;

	check_config(TEST_NODES);

	if (access(PATH_HUGEPAGES, F_OK))
		tst_brk(TCONF, "Huge page not supported");

	pgsz = (int)get_page_size();
	SAFE_FILE_LINES_SCANF(PATH_MEMINFO, "Hugepagesize: %d", &hpsz);
	hpsz *= 1024;

	SAFE_FILE_LINES_SCANF(PATH_MEMINFO, "MemFree: %d", &memfree);
	memfree *= 1024;
	if (4 * hpsz > memfree)
		tst_brk(TBROK, "Not enough free RAM");

	SAFE_FILE_SCANF(PATH_NR_HUGEPAGES, "%ld", &orig_hugepages);
	SAFE_FILE_PRINTF(PATH_NR_HUGEPAGES, "%ld", orig_hugepages + 4);

	ret = get_allowed_nodes(NH_MEMS, TEST_NODES, &node1, &node2);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes: %d", ret);
}

static void cleanup(void)
{
	SAFE_FILE_PRINTF(PATH_NR_HUGEPAGES, "%ld", orig_hugepages);
}

static struct tst_test test = {
	.tid = "move_pages12",
	.min_kver = "2.6.32",
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
};

#else
	TST_TEST_TCONF("move_pages support not found");
#endif
