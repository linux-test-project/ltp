/*
 * $Copyright: $
 * Copyright (c) 1984-2000
 * Sequent Computer Systems, Inc.   All rights reserved.
 *
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

/*
 * Test Name: getresgid01
 *
 * Test Description:
 *  Verify that getresgid() will be successful to get the real, effective
 *  and saved user id of the calling process.
 *
 * Expected Result:
 *  getresgid() should return with 0 value and the real/effective/saved
 *  user ids should be equal to that of calling process.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  getresgid01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

extern int getresgid(gid_t *, gid_t *, gid_t *);

char *TCID = "getresgid01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
gid_t pr_gid, pe_gid, ps_gid;	/* calling process real/effective/saved gid */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	gid_t real_gid,		/* real/eff./saved user id from getresgid() */
	 eff_gid, sav_gid;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call getresgid() to get the real/effective/saved
		 * user id's of the calling process.
		 */
		TEST(getresgid(&real_gid, &eff_gid, &sav_gid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "getresgid() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Verify the real/effective/saved gid values returned
			 * by getresgid with the expected values.
			 */
			if ((real_gid != pr_gid) || (eff_gid != pe_gid) ||
			    (sav_gid != ps_gid)) {
				tst_resm(TFAIL, "real:%d, effective:%d, "
					 "saved-user:%d ids differ",
					 real_gid, eff_gid, sav_gid);
			} else {
				tst_resm(TPASS, "Functionality of getresgid() "
					 "successful");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the real/effective/saved user id of the calling process.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Real user-id of the calling process */
	pr_gid = getgid();

	/* Effective user-id of the calling process */
	pe_gid = getegid();

	/* Saved user-id of the calling process */
	ps_gid = getegid();

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}