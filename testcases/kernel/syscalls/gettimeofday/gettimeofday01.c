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
 *	gettimeofday01.c
 *
 * DESCRIPTION
 *	Testcase to check that gettimeofday(2) sets errno to EFAULT.
 *
 * ALGORITHM
 *	Call gettimeofday() with an invalid buffer, and expect EFAULT to be
 *	set in errno.
 *
 * USAGE:  <for command-line>
 *  gettimeofday01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <sys/time.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <sys/syscall.h>
#include <unistd.h>

#define gettimeofday(a,b)  syscall(__NR_gettimeofday,a,b)

char *TCID = "gettimeofday01";
#if !defined UCLINUX

int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { EFAULT, 0 };

void cleanup(void);
void setup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ret;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(gettimeofday((void *)-1, (void *)-1));

		/* gettimeofday returns an int, so we need to turn the long
		 * TEST_RETURN into an int to test with */
		ret = TEST_RETURN;
		if (ret != -1) {
			tst_resm(TFAIL,
				 "call succeeded unexpectedly (got back %i, wanted -1)",
				 ret);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);
		if (TEST_ERRNO == EFAULT)
			tst_resm(TPASS,
				 "gettimeofday(2) set the errno EFAULT correctly");
		else
			tst_resm(TFAIL,
				 "gettimeofday(2) didn't set errno to EFAULT, errno=%i (%s)",
				 errno, strerror(errno));
	}
	cleanup();

	 /*NOTREACHED*/ return (0);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
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

int TST_TOTAL = 0;		/* Total number of test cases. */

int main(void)
{
	tst_resm(TPASS, "gettimeofday EFAULT check disabled on uClinux");
	tst_exit();
	return 0;
}

#endif
