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
 *	getgid02.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of getegid().
 *
 * ALGORITHM
 *	call setup
 *	loop if that option was specified
 *	Execute getegid() call using TEST macro
 *	if not expected value
 *	   break remaining tests and cleanup
 *	if STD_FUNCTIONAL_TEST
 *	   Execute geteuid() call
 *	   Execute getpwduid() call
 *	   if the passwd entry is NULL
 *	      break the remaining tests and cleanup
 *	   else if pwent->pw_gid != TEST_RETURN
 *	      print failure message
 *	   else
 *	      print pass message
 *	else
 *	   print pass message
 *      call cleanup
 *
 * USAGE:  <for command-line>
 *  getgid02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include <pwd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID= "getgid02";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int lc;                         /* loop counter */
	char *msg;                      /* message returned from parse_opts */
	int euid;
	struct passwd *pwent;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	setup();                        /* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(getegid());

		if (TEST_RETURN < 0) {
			tst_brkm(TBROK, cleanup, "This should never happen");
		}

		if (STD_FUNCTIONAL_TEST) {
			euid = geteuid();

			pwent = getpwuid(euid);

			if (pwent == NULL) {
				tst_brkm(TBROK, cleanup, "geteuid() returned "
					 "unexpected value %d", euid);
			} else {
				if (pwent->pw_gid != TEST_RETURN) {
					tst_resm(TFAIL, "getegid() return value"
						" %d unexpected - expected %d",
						TEST_RETURN, pwent->pw_gid);
				} else {
					tst_resm(TPASS, "effective group id %d "
						 "is correct", TEST_RETURN);
				}
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}
	cleanup();

	/*NOTREACHED*/
	return(0);
}


/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void
cleanup()
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
