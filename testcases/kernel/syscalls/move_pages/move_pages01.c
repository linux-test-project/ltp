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
 *	move_pages01.c
 *
 * DESCRIPTION
 *      Test retrieval of NUMA node
 *
 * ALGORITHM
 *      1. Allocate pages in NUMA nodes A and B.
 *      2. Use move_pages() to retrieve the NUMA node of the pages.
 *      3. Check if the NUMA nodes reported are correct.
 *
 * USAGE:  <for command-line>
 *      move_pages01 [-c n] [-i n] [-I x] [-P x] [-t]
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
#include "usctest.h"
#include "move_pages_support.h"

#define TEST_PAGES 2
#define TEST_NODES TEST_PAGES

void setup(void);
void cleanup(void);

char *TCID = "move_pages01";
int TST_TOTAL = 1;

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
	int lc;			/* loop counter */

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[TEST_PAGES] = { 0 };
		int status[TEST_PAGES];
		int ret;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = alloc_pages_linear(pages, TEST_PAGES);
		if (ret == -1)
			continue;

		ret = numa_move_pages(0, TEST_PAGES, pages, NULL, status, 0);
		TEST_ERRNO = errno;

		if (ret != 0) {
			tst_resm(TFAIL, "retrieving NUMA nodes failed");
			free_pages(pages, TEST_PAGES);
			continue;
		}

		verify_pages_linear(pages, status, TEST_PAGES);

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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	check_config(TEST_NODES);
	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

 }
