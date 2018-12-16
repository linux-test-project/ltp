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
 *	move_pages02.c
 *
 * DESCRIPTION
 *      Test movement of pages mapped by a process.
 *
 * ALGORITHM
 *      1. Allocate pages in NUMA node A.
 *      2. Use move_pages() to move the pages to NUMA node B.
 *      3. Retrieve the NUMA nodes of the moved pages.
 *      4. Check if all pages are in node B.
 *
 * USAGE:  <for command-line>
 *      move_pages02 [-c n] [-i n] [-I x] [-P x] [-t]
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

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"
#include "move_pages_support.h"

#define TEST_PAGES 2
#define TEST_NODES 2

void setup(void);
void cleanup(void);

char *TCID = "move_pages02";
int TST_TOTAL = 1;

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

		/* reset tst_count in case we are looping */
		tst_count = 0;

		ret = alloc_pages_on_node(pages, TEST_PAGES, from_node);
		if (ret == -1)
			continue;

		for (i = 0; i < TEST_PAGES; i++)
			nodes[i] = to_node;

		ret =
		    numa_move_pages(0, TEST_PAGES, pages, nodes, status,
				    MPOL_MF_MOVE);
		if (ret < 0) {
			tst_resm(TFAIL|TERRNO, "move_pages failed");
			free_pages(pages, TEST_PAGES);
			continue;
		} else if (ret > 0) {
			tst_resm(TINFO, "move_pages() returned %d\n", ret);
		}

		for (i = 0; i < TEST_PAGES; i++)
			*((char *)pages[i]) = 0xAA;

		verify_pages_on_node(pages, status, TEST_PAGES, to_node);

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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

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
