/*
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
 * Test Name: msync03
 *
 * Test Description:
 *  Verify that, msync() fails, when the region to synchronize, is outside
 *  the address space of the process.
 *
 * Expected Result:
 *  msync() should fail with a return value of -1, and errno should be
 *  set to EINVAL.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *      if errno set == expected errno
 *              Issue sys call fails with expected return value and errno.
 *      Otherwise,
 *              Issue sys call fails with unexpected errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  msync03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

char *TCID = "msync03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

void *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */

int exp_enos[] = { EINVAL, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(msync(addr, page_sz, MS_ASYNC));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "msync() returns unexpected "
				 "value %ld, expected:-1", TEST_RETURN);
			continue;
		}

		if (errno == EINVAL)
			tst_resm(TPASS, "msync failed with EINVAL as expected");
		else
			tst_resm(TFAIL|TERRNO, "msync failed unexpectedly");
	}

	cleanup();
	tst_exit();
}

void setup()
{
	struct rlimit brkval;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if ((page_sz = getpagesize()) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "getpagesize failed");

	getrlimit(RLIMIT_DATA, &brkval);

	addr = (void*) brkval.rlim_max;
}

void cleanup()
{
	TEST_CLEANUP;

}