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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	move_pages04.c
 *
 * DESCRIPTION
 *      Failure when page does not exit.
 *
 * ALGORITHM
 *
 *      1. Pass a page that does not exit as one of the page addresses
 *         to move_pages().
 *      2. Check if the corresponding status is set to -ENOENT.
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
#include "usctest.h"
#include "move_pages_support.h"

#define TEST_PAGES 2
#define TEST_NODES 2
#define TOUCHED_PAGES 1
#define UNTOUCHED_PAGE (TEST_PAGES - 1)

void setup(void);
void cleanup(void);

char *TCID = "move_pages04";
int TST_TOTAL = 1;

typedef void (*sighandler_t) (int);

int main(int argc, char **argv)
{
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

#if HAVE_NUMA_MOVE_PAGES
	unsigned int i;
	int lc;			/* loop counter */
	unsigned int from_node = 0;
	unsigned int to_node = 1;

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[TEST_PAGES] = { 0 };
		int nodes[TEST_PAGES];
		int status[TEST_PAGES];
		int ret;
		unsigned long onepage = get_page_size();

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = alloc_pages_on_node(pages, TOUCHED_PAGES, from_node);
		if (ret == -1)
			continue;

		/* Allocate page and do not touch it. */
		pages[UNTOUCHED_PAGE] = numa_alloc_onnode(onepage, from_node);
		if (pages[UNTOUCHED_PAGE] == NULL) {
			tst_resm(TBROK, "failed allocating page on node %d",
				 from_node);
			goto err_free_pages;
		}

		for (i = 0; i < TEST_PAGES; i++)
			nodes[i] = to_node;

		ret = numa_move_pages(0, TEST_PAGES, pages, nodes,
				      status, MPOL_MF_MOVE);
		TEST_ERRNO = errno;
		if (ret == -1) {
			tst_resm(TFAIL | TERRNO, "move_pages unexpectedly failed");
			goto err_free_pages;
		}

		if (status[UNTOUCHED_PAGE] == -ENOENT)
			tst_resm(TPASS, "status[%d] set to expected -ENOENT",
				 UNTOUCHED_PAGE);
		else
			tst_resm(TFAIL, "status[%d] is %d", UNTOUCHED_PAGE,
				 status[UNTOUCHED_PAGE]);

		err_free_pages:
		    /* This is capable of freeing both the touched and
		     * untouched pages.
		     */
		    free_pages(pages, TEST_PAGES);
	}
#else
	tst_resm(TCONF, "move_pages support not found.");
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
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

 }