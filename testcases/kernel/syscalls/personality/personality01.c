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
 * NAME
 *	personality01.c
 *
 * DESCRIPTION
 *	personality01 - Check that we can set the personality for a process.
 *
 * CALLS
 *	personality()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if the return value is the previous personality
 *	  value
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  personality01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/personality.h>
#undef personality

extern int personality(unsigned long);

void cleanup(void);
void setup(void);

char *TCID = "personality01";
int TST_TOTAL = 13;

int pers[] = { PER_LINUX, PER_LINUX_32BIT, PER_SVR4, PER_SVR3, PER_SCOSVR3,
	PER_WYSEV386, PER_ISCR4, PER_BSD, PER_XENIX, PER_LINUX32,
	PER_IRIX32, PER_IRIXN32, PER_IRIX64
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i, start_pers;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();		/* global setup */

	start_pers = personality(PER_LINUX);
	if (start_pers == -1) {
		printf("personality01:  Test Failed\n");
		exit(-1);
	}

	/* The following checks the looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Start looping through our series of personalities and
		 * make sure that we can change to each one and the return
		 * value is the previous one.
		 */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(personality(pers[i]));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed - "
					 "errno = %d - %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				/*
				 * check to make sure that the return value
				 * is the previous personality in our list.
				 *
				 * i == 0 is a special case since the return
				 * value should equal pers[0].
				 */
				if (TEST_RETURN == pers[i == 0 ? 0 : i - 1]) {
					tst_resm(TPASS, "personality set "
						 "correctly");
				} else {
					tst_resm(TFAIL, "returned persona "
						 "was not expected");
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}
		/*
		 * set our personality back to PER_LINUX
		 */
		if (personality(start_pers) == -1) {
			tst_brkm(TBROK, cleanup, "failed personality reset");
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
}