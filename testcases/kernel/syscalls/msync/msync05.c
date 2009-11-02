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
 * Test Name: msync05
 *
 * Test Description:
 *  Verify that, msync() fails, when the region to synchronize, was not
 *  mapped.
 *
 * Expected Result:
 *  msync() should fail with a return value of -1, and set errno ENOMEM.
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
 *  msync05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	03/2002 Paul Larson: expected error should be ENOMEM not EFAULT
 *
 * RESTRICTIONS:
 *  None.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

char *TCID = "msync05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */

int exp_enos[] = { ENOMEM, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

#if !defined(UCLINUX)
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call msync to synchronize the specified memory region
		 * which was not mapped previously.
		 */
		TEST(msync(addr, page_sz, MS_SYNC));

		/* Check for the return value of msync() */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "msync() returns unexpected value %ld, "
				 "expected:-1", TEST_RETURN);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/*
		 * Verify whether expected errno is
		 * set (ENOMEM).
		 */
		if (errno == ENOMEM) {
			tst_resm(TPASS, "memory region not mapped, errno:%d",
				 TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "msync() fails, unexpected errno:%d, "
				 "expected: ENOMEM", TEST_ERRNO);
		}
	}			/* End for TEST_LOOPING */

	/* Call test cleanup to exit the test */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *
 * Get system page size,
 * Set the virtual address points to some high address which is not mapped.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TBROK, cleanup,
			 "getpagesize() fails to get system page size");
	}

	/*
	 * Set the virtual address point to some high address
	 * which is not mapped.
	 */
	addr = (char *)get_high_address();
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Exit the program with proper exit code.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */
