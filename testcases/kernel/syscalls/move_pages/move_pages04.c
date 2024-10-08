/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 *
 *   Based on testcases/kernel/syscalls/waitpid/waitpid01.c
 *   Original copyright message:
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	move_pages04.c
 *
 * DESCRIPTION
 *      Failure when the memory area is not valid, no page is mapped yet or
 *      the shared zero page is mapped.
 *
 * ALGORITHM
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
 * [1]
 * d899844e9c98 "mm: fix status code which move_pages() returns for zero page"
 * [2]
 * 7dff875c9436 "mm/migrate: convert add_page_for_migration() from follow_page() to folio_walk"
 *
 * USAGE:  <for command-line>
 *      move_pages04 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	05/2008 Vijay Kumar
 *		Initial Version.
 *
 * Restrictions
 *	None
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "test.h"
#include "move_pages_support.h"

#define TEST_PAGES 4
#define TEST_NODES 2
#define TOUCHED_PAGES 1
#define NO_PAGE TOUCHED_PAGES
#define ZERO_PAGE (NO_PAGE + 1)
#define INVALID_PAGE (ZERO_PAGE + 1)

void setup(void);
void cleanup(void);

char *TCID = "move_pages04";
int TST_TOTAL = 1;

typedef void (*sighandler_t) (int);

int main(int argc, char **argv)
{

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

#ifdef HAVE_NUMA_V2
	unsigned int i;
	int lc;
	unsigned int from_node;
	unsigned int to_node;
	int ret;

	ret = get_allowed_nodes(NH_MEMS, 2, &from_node, &to_node);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "get_allowed_nodes: %d", ret);

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[TEST_PAGES] = { 0 };
		int nodes[TEST_PAGES];
		int status[TEST_PAGES];
		unsigned long onepage = get_page_size();
		char tmp;

		/* reset tst_count in case we are looping */
		tst_count = 0;

		ret = alloc_pages_on_node(pages, TOUCHED_PAGES, from_node);
		if (ret == -1)
			continue;

		/*
		 * Allocate memory and do not touch it. Consequently, no
		 * page will be faulted in / mapped into the page tables.
		 */
		pages[NO_PAGE] = numa_alloc_onnode(onepage, from_node);
		if (pages[NO_PAGE] == NULL) {
			tst_resm(TBROK, "failed allocating memory on node %d",
				 from_node);
			goto err_free_pages;
		}

		/*
		 * Allocate memory, read from it, but do not write to it. This
		 * will populate the shared zeropage.
		 */
		pages[ZERO_PAGE] = numa_alloc_onnode(onepage, from_node);
		if (pages[ZERO_PAGE] == NULL) {
			tst_resm(TBROK, "failed allocating memory on node %d",
				 from_node);
			goto err_free_pages;
		}
		/* Make the compiler not optimize-out the read. */
		tmp = *((char *)pages[ZERO_PAGE]);
		asm volatile("" : "+r" (tmp));

		/*
		 * Temporarily allocate memory and free it immediately. Do this
		 * as the last step so the area won't get reused before we're
		 * done.
		 */
		pages[INVALID_PAGE] = numa_alloc_onnode(onepage, from_node);
		if (pages[INVALID_PAGE] == NULL) {
			tst_resm(TBROK, "failed allocating memory on node %d",
				 from_node);
			goto err_free_pages;
		}
		numa_free(pages[INVALID_PAGE], onepage);

		for (i = 0; i < TEST_PAGES; i++)
			nodes[i] = to_node;

		ret = numa_move_pages(0, TEST_PAGES, pages, nodes,
				      status, MPOL_MF_MOVE);
		if (ret == -1) {
			tst_resm(TFAIL | TERRNO,
				 "move_pages unexpectedly failed");
			goto err_free_pages;
		} else if (ret > 0) {
			tst_resm(TINFO, "move_pages() returned %d", ret);
		}

		if (status[NO_PAGE] == -ENOENT) {
			tst_resm(TPASS, "status[%d] has expected value",
				 NO_PAGE);
		} else {
			tst_resm(TFAIL, "status[%d] is %s, expected %s",
				NO_PAGE,
				tst_strerrno(-status[NO_PAGE]),
				tst_strerrno(ENOENT));
		}

		if (status[ZERO_PAGE] == -EFAULT) {
			tst_resm(TPASS, "status[%d] has expected value",
				 ZERO_PAGE);
		} else {
			tst_resm(TFAIL, "status[%d] is %s, expected %s",
				ZERO_PAGE,
				tst_strerrno(-status[ZERO_PAGE]),
				tst_strerrno(EFAULT));
		}

		if (status[INVALID_PAGE] == -EFAULT) {
			tst_resm(TPASS, "status[%d] has expected value",
				 INVALID_PAGE);
		} else {
			tst_resm(TFAIL, "status[%d] is %s, expected %s",
				INVALID_PAGE,
				tst_strerrno(-status[INVALID_PAGE]),
				tst_strerrno(EFAULT));
		}

err_free_pages:
		/* Memory for the invalid page was already freed. */
		pages[INVALID_PAGE] = NULL;
		/* This is capable of freeing all memory we allocated. */
		free_pages(pages, TEST_PAGES);
	}
#else
	tst_resm(TCONF, NUMA_ERROR_MSG);
#endif

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	check_config(TEST_NODES);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion
 */
void cleanup(void)
{

}
