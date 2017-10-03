/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: adjtimex02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 6
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) adjtimex(2) fails with errno set to EFAULT if buf does
 *	   not point to writable memory
 *	2) adjtimex(2) fails with errno set to EINVAL if an attempt
 *	   is  made  to set buf.tick to a value < 900000/HZ
 *	3) adjtimex(2) fails with errno set to EINVAL if an attempt
 *	   is  made  to set buf.tick to a value > 1100000/HZ
 *	4) adjtimex(2) fails with errno set to EINVAL if an attempt
 *	   is  made  to set buf.offset to a value > 512000L
 *	   (This test case will be executed only if the kernel version
 *	    is 2.6.25 or below)
 *	5) adjtimex(2) fails with errno set to EINVAL if an attempt
 *	   is  made  to set buf.offset to a value < 512000L
 *	   (This test case will be executed only if the kernel version
 *	    is 2.6.25 or below)
 *	6) adjtimex(2) fails with errno set to EPERM if buf.mode is
 *	   non-zero and the user is not super-user.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Save current parameters in tim_save
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Call test case specific setup if needed
 * 	  call adjtimex with saved timex structure
 *	  Check return value is between 0 & 5
 *		Test passed
 *	  Otherwise
 *		Test failed
 *	  Call test case specific cleanup if needed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * adjtimex02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug causing failure in sys/timex.h */
#define __THROW
#endif

#include <errno.h>
#include <sys/timex.h>
#include <unistd.h>
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"

#define SET_MODE ( ADJ_OFFSET | ADJ_FREQUENCY | ADJ_MAXERROR | ADJ_ESTERROR | \
	ADJ_STATUS | ADJ_TIMECONST | ADJ_TICK )

static void setup(void);
static int setup2(void);
static int setup3(void);
static int setup4(void);
static int setup5(void);
static int setup6(void);
static void cleanup(void);
static void cleanup6(void);

char *TCID = "adjtimex02";

static int hz;			/* HZ from sysconf */

static struct timex tim_save;
static struct timex buff;

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct test_cases_t {
	struct timex *buffp;
	int (*setup) ();
	void (*cleanup) ();
	int exp_errno;
} test_cases[] = {
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	{
	(struct timex *)-1, NULL, NULL, EFAULT},
#endif
	{
	&buff, setup2, NULL, EINVAL}, {
	&buff, setup3, NULL, EINVAL}, {
	&buff, setup4, NULL, EINVAL}, {
	&buff, setup5, NULL, EINVAL}, {
	&buff, setup6, cleanup6, EPERM}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			/*
			 * since Linux 2.6.26, if buf.offset value is outside
			 * the acceptable range, it is simply normalized instead
			 * of letting the syscall fail. so just skip this test
			 * case.
			 */
			if ((i == 3 || i == 4) && tst_kvercmp(2, 6, 25) > 0) {
				tst_resm(TCONF, "this kernel normalizes buf."
					 "offset value if it is outside"
					 " the acceptable range.");
				continue;
			}

			buff = tim_save;
			buff.modes = SET_MODE;
			if ((test_cases[i].setup) && (test_cases[i].setup())) {
				tst_resm(TWARN, "setup() failed, skipping"
					 " this test case");
				continue;
			}

			/* Call adjtimex(2) */
			TEST(adjtimex(test_cases[i].buffp));

			if ((TEST_RETURN == -1) && (TEST_ERRNO ==
						    test_cases[i].exp_errno)) {
				tst_resm(TPASS | TTERRNO,
					 "Test Passed, adjtimex() returned -1");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "Test Failed, adjtimex() returned %ld",
					 TEST_RETURN);
			}
			if (test_cases[i].cleanup) {
				test_cases[i].cleanup();
			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{
	tst_require_root();

	tim_save.modes = 0;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the HZ from sysconf */
	hz = SAFE_SYSCONF(NULL, _SC_CLK_TCK);

	TEST_PAUSE;

	/* Save current parameters in tim_save */
	if ((adjtimex(&tim_save)) == -1) {
		tst_brkm(TBROK, NULL, "Failed to save current parameters");
	}
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{

	tim_save.modes = SET_MODE;
	/* Restore saved parameters */
	if ((adjtimex(&tim_save)) == -1) {
		tst_resm(TWARN, "Failed to restore saved parameters");
	}
}

int setup2(void)
{
	buff.tick = 900000 / hz - 1;
	return 0;
}

int setup3(void)
{
	buff.tick = 1100000 / hz + 1;
	return 0;
}

int setup4(void)
{
	buff.offset = 512000L + 1;
	return 0;
}

int setup5(void)
{
	buff.offset = (-1) * (512000L) - 1;
	return 0;
}

int setup6(void)
{
	/* Switch to nobody user for correct error code collection */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "\"nobody\" user not present");
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN | TERRNO, "seteuid(%d) failed", ltpuser->pw_uid);
		return 1;
	}
	return 0;
}

void cleanup6(void)
{
	/* Set effective user id back to root */
	SAFE_SETEUID(cleanup, 0);
}
