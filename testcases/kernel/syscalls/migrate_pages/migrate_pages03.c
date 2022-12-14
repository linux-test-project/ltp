// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 FUJITSU LIMITED. All rights reserved.
 * Author(s): Xiao Yang <yangx.jy@cn.fujitsu.com>
 *            Jie Fei <feij.fnst@cn.fujitsu.com>
 */

/*
 * Description:
 * This is a regression test for ksm page migration which is miscalculated.
 *
 * The kernel bug has been fixed by:
 *
 * commit 4b0ece6fa0167b22c004ff69e137dc94ee2e469e
 * Author: Naoya Horiguchi <n-horiguchi@ah.jp.nec.com>
 * Date:   Fri Mar 31 15:11:44 2017 -0700
 *
 *     mm: migrate: fix remove_migration_pte() for ksm pages
 */

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/mmap.h"
#include "ksm_helper.h"
#include "numa_helper.h"
#include "migrate_pages_common.h"

#ifdef HAVE_NUMA_V2
#define N_PAGES 20
#define N_LOOPS 600
#define TEST_NODES 2

static int orig_ksm_run = -1;
static unsigned int page_size;
static void *test_pages[N_PAGES];
static int num_nodes, max_node;
static int *nodes;
static unsigned long *new_nodes[2];
static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void setup(void)
{
	int n;
	unsigned long nodemask_size;

	if (access(PATH_KSM, F_OK))
		tst_brk(TCONF, "KSM configuration was not enabled");

	if (get_allowed_nodes_arr(NH_MEMS, &num_nodes, &nodes) < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes() failed");

	if (num_nodes < TEST_NODES) {
		tst_brk(TCONF, "requires NUMA with at least %d node",
			TEST_NODES);
	}

	ltpuser = SAFE_GETPWNAM(nobody_uid);

	max_node = LTP_ALIGN(get_max_node(), sizeof(unsigned long) * 8);
	nodemask_size = max_node / 8;
	new_nodes[0] = SAFE_MALLOC(nodemask_size);
	new_nodes[1] = SAFE_MALLOC(nodemask_size);
	memset(new_nodes[0], 0, nodemask_size);
	memset(new_nodes[1], 0, nodemask_size);
	set_bit(new_nodes[0], nodes[0], 1);
	set_bit(new_nodes[1], nodes[1], 1);

	page_size = getpagesize();

	for (n = 0; n < N_PAGES; n++) {
		test_pages[n] = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
					  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (madvise(test_pages[n], page_size, MADV_MERGEABLE)) {
			if (errno == EINVAL) {
				tst_brk(TCONF | TERRNO, "madvise() didn't "
					"support MADV_MERGEABLE");
			}

			tst_brk(TBROK | TERRNO,
				"madvise(MADV_MERGEABLE) failed");
		}

		if (mbind(test_pages[n], page_size, MPOL_BIND, new_nodes[0],
			  max_node, 0))
			tst_brk(TBROK | TERRNO, "mbind(MPOL_BIND) failed");

		memset(test_pages[n], 0, page_size);
	}

	SAFE_FILE_SCANF(PATH_KSM "run", "%d", &orig_ksm_run);
	SAFE_FILE_PRINTF(PATH_KSM "run", "%d", 1);
	wait_ksmd_full_scan();
}

static void cleanup(void)
{
	int n;

	for (n = 0; n < N_PAGES; n++) {
		if (test_pages[n])
			SAFE_MUNMAP(test_pages[n], page_size);
	}

	free(new_nodes[0]);
	free(new_nodes[1]);

	if (orig_ksm_run != -1)
		SAFE_FILE_PRINTF(PATH_KSM "run", "%d", orig_ksm_run);
}

static void migrate_test(void)
{
	int loop, i, ret;

	SAFE_SETEUID(ltpuser->pw_uid);
	for (loop = 0; loop < N_LOOPS; loop++) {
		i = loop % 2;
		ret = tst_syscall(__NR_migrate_pages, 0, max_node,
				   new_nodes[i], new_nodes[i ? 0 : 1]);
		if (ret < 0) {
			tst_res(TFAIL | TERRNO, "migrate_pages() failed");
			return;
		}

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Out of runtime, exiting...");
			break;
		}
	}
	SAFE_SETEUID(0);

	tst_res(TPASS, "migrate_pages() passed");
}

static struct tst_test test = {
	.max_runtime = 300,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = migrate_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "4b0ece6fa016"},
		{}
	}
};

#else
	TST_TEST_TCONF("require libnuma >= 2 and it's development packages");
#endif
