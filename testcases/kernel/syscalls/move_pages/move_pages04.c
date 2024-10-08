// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Verify that move_pages() properly reports failures when the memory area is
 * not valid, no page is mapped yet or the shared zero page is mapped.
 *
 * [Algorithm]
 *
 *      1. Pass the address of a valid memory area where no page is
 *         mapped yet (not read/written), the address of a valid memory area
 *         where the shared zero page is mapped (read, but not written to)
 *         and the address of an invalid memory area as page addresses to
 *         move_pages().
 *      2. Check if the corresponding status for "no page mapped" is set to
 *         -ENOENT. Note that kernels >= 4.3 [1] and < 6.12 [2] wrongly returned
 *         -EFAULT by accident.
 *      3. Check if the corresponding status for "shared zero page" is set to:
 *         -EFAULT. Note that kernels < 4.3 [1] wrongly returned -ENOENT.
 *      4. Check if the corresponding status for "invalid memory area" is set
 *         to -EFAULT.
 *
 *   [1] d899844e9c98 "mm: fix status code which move_pages() returns for zero page"
 *   [2] 7dff875c9436 "mm/migrate: convert add_page_for_migration() from follow_page() to folio_walk"
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "tst_test.h"
#include "move_pages_support.h"

#define TEST_PAGES 4
#define TEST_NODES 2
#define TOUCHED_PAGES 1
#define NO_PAGE TOUCHED_PAGES
#define ZERO_PAGE (NO_PAGE + 1)
#define INVALID_PAGE (ZERO_PAGE + 1)

static void run(void)
{
#ifdef HAVE_NUMA_V2
	unsigned int i;
	unsigned int from_node;
	unsigned int to_node;
	int ret;
	void *pages[TEST_PAGES] = { 0 };
	int nodes[TEST_PAGES];
	int status[TEST_PAGES];
	unsigned long onepage = get_page_size();
	char tmp;

	ret = get_allowed_nodes(NH_MEMS, 2, &from_node, &to_node);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes: %d", ret);

	ret = alloc_pages_on_node(pages, TOUCHED_PAGES, from_node);
	if (ret == -1)
		tst_brk(TBROK, "failed allocating memory on node %d",
			from_node);

	/*
	 * Allocate memory and do not touch it. Consequently, no
	 * page will be faulted in / mapped into the page tables.
	 */
	pages[NO_PAGE] = numa_alloc_onnode(onepage, from_node);
	if (pages[NO_PAGE] == NULL)
		tst_brk(TBROK, "failed allocating memory on node %d",
			from_node);

	/*
	 * Allocate memory, read from it, but do not write to it. This
	 * will populate the shared zeropage.
	 */
	pages[ZERO_PAGE] = numa_alloc_onnode(onepage, from_node);
	if (pages[ZERO_PAGE] == NULL)
		tst_brk(TBROK, "failed allocating memory on node %d",
			from_node);
	/* Make the compiler not optimize-out the read. */
	tmp = *((char *)pages[ZERO_PAGE]);
	asm volatile("" : "+r" (tmp));

	/*
	 * Temporarily allocate memory and free it immediately. Do this
	 * as the last step so the area won't get reused before we're
	 * done.
	 */
	pages[INVALID_PAGE] = numa_alloc_onnode(onepage, from_node);
	if (pages[INVALID_PAGE] == NULL)
		tst_brk(TBROK, "failed allocating memory on node %d",
			from_node);
	numa_free(pages[INVALID_PAGE], onepage);

	for (i = 0; i < TEST_PAGES; i++)
		nodes[i] = to_node;

	ret = numa_move_pages(0, TEST_PAGES, pages, nodes,
			      status, MPOL_MF_MOVE);
	if (ret == -1) {
		tst_res(TFAIL | TERRNO,
			"move_pages unexpectedly failed");
		goto err_free_pages;
	} else if (ret > 0) {
		tst_res(TINFO, "move_pages() returned %d", ret);
	}

	if (status[NO_PAGE] == -ENOENT) {
		tst_res(TPASS, "status[%d] has expected value",
			NO_PAGE);
	} else {
		tst_res(TFAIL, "status[%d] is %s, expected %s",
			NO_PAGE,
			tst_strerrno(-status[NO_PAGE]),
			tst_strerrno(ENOENT));
	}

	if (status[ZERO_PAGE] == -EFAULT) {
		tst_res(TPASS, "status[%d] has expected value",
			ZERO_PAGE);
	} else {
		tst_res(TFAIL, "status[%d] is %s, expected %s",
			ZERO_PAGE,
			tst_strerrno(-status[ZERO_PAGE]),
			tst_strerrno(EFAULT));
	}

	if (status[INVALID_PAGE] == -EFAULT) {
		tst_res(TPASS, "status[%d] has expected value",
			INVALID_PAGE);
	} else {
		tst_res(TFAIL, "status[%d] is %s, expected %s",
			INVALID_PAGE,
			tst_strerrno(-status[INVALID_PAGE]),
			tst_strerrno(EFAULT));
	}

err_free_pages:
	/* Memory for the invalid page was already freed. */
	pages[INVALID_PAGE] = NULL;
	/* This is capable of freeing all memory we allocated. */
	free_pages(pages, TEST_PAGES);
#else
	tst_res(TCONF, NUMA_ERROR_MSG);
#endif
}

void setup(void)
{
	check_config(TEST_NODES);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d899844e9c98"},
		{"linux-git", "7dff875c9436"},
		{}
	}
};
