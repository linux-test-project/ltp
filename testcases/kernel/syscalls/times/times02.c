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
 *	times02.c
 *
 * DESCRIPTION
 *	Testcase to test that times() sets errno correctly
 *
 * ALGORITHM
 *	block1:	Pass an invalid address as the "tms" structure, and expect
 *		that times() would return EFAULT.
 *
 * USAGE:  <for command-line>
 *	times02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	NONE
 */

#include <sys/times.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>

char *TCID = "times02";
int TST_TOTAL = 1;
extern int Tst_count;
int exp_enos[]={EFAULT, 0};

void setup(void);
void cleanup(void);

#if !defined(UCLINUX)

int main(int argc, char **argv)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *)NULL, NULL)) !=
		(char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOTREACHED*/
	}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(times((void *)-1));
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (errno != EFAULT) {
				tst_resm(TFAIL, "Expected EFAULT, got %d",
						errno);
			} else {
				tst_resm(TPASS, "Received EFAULT as expected");
			}
				
		} else {
			tst_resm(TFAIL, "times(2) failed to FAIL");
		}

	}
	cleanup();
	/*NOTREACHED*/

  return(0);

}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void
cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}
