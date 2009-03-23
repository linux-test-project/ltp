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
 * 	settimeofday02.c
 *
 * DESCRIPTION
 *	Testcase to check that settimeofday() sets errnos correctly.
 *
 * ALGORITHM
 *	Setup:
 *	  Setup signal handling.
 *	  Check that we are not root.
 *	  Setup expected errnos.
 *	  Pause for SIGUSER1 if option specified.
 *	  Save the current time values.
 *	Loop if the proper options are given.
 *	  Call settimeofday with an invalid "buf" address and verify that
 *		errno is set to EFAULT.
 *	  Call settimeofday as a non-root user.  Verify that the call fails
 *		and errno is set to EPERM.
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given.
 *
 * USAGE:  <for command-line>
 *	settimeofday02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	Must not be run as root
 */

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <pwd.h>

#define	VAL_SEC		100
#define	VAL_MSEC	100

char *TCID = "settimeofday02";
int TST_TOTAL = 1;
int exp_enos[] = { EFAULT, EPERM, 0 };
struct timeval tp;
time_t save_tv_sec, save_tv_usec;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

extern int Tst_count;

void setup(void);
void cleanup(void);
void restore_time(void);

#if !defined(UCLINUX)

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(settimeofday((void *)-1, NULL));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "settimeofday(2) failed to FAIL");
			restore_time();
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO != EFAULT) {
				tst_resm(TFAIL, "Expected EFAULT got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "Received expected errno");
			}
		}

		tp.tv_sec = VAL_SEC;
		tp.tv_usec = VAL_MSEC;
		TEST(settimeofday(&tp, NULL));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "settimeofday(2) failed to FAIL");
			restore_time();
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO != EPERM) {
				tst_resm(TFAIL, "Expected EPERM got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "Received expected errno");
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

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
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
		tst_exit();
	 /*NOTREACHED*/}

	/* Switch to nobody user for correct error code collection */
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	/* Save the current time values */
	if ((gettimeofday(&tp, (struct timezone *)&tp)) == -1) {
		tst_brkm(TBROK, cleanup, "gettimeofday failed. "
			 "errno=%d", errno);
	 /*NOTREACHED*/}
	save_tv_sec = tp.tv_sec;
	save_tv_usec = tp.tv_usec;
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
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
 /*NOTREACHED*/}

void restore_time(void)
{
	/* restore the original time values. */
	tp.tv_sec = save_tv_sec;
	tp.tv_usec = save_tv_usec;
	if ((settimeofday(&tp, NULL)) == -1) {
		tst_resm(TWARN, "FATAL COULD NOT RESET THE CLOCK");
		tst_resm(TFAIL, "Error Setting Time, errno=%d", errno);
	}
}
