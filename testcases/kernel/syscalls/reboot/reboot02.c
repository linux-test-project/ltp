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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: reboot02
 *
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test checking for basic error conditions
 *    				 for reboot(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case checks whether reboot(2) system call  returns
 *	appropriate error number for invalid
 *	flag parameter or invalid user.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  For testing error on invalid user, change the effective uid
 *
 * 	Test:
 *	  Loop if the proper options are given.
 *	  Execute system call with invaid flag parameter
 *	  and then for invalid user
 *	  Check return code, if system call fails with errno == expected errno
 *		Issue syscall passed with expected errno
 *	  Otherwise,
 *	  Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	  Do cleanup for the test.
 * 	 $
 * USAGE:  <for command-line>
 *  reboot02 [-c n] [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where
 *  	-c n: run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *for lib4 and lib5 reboot(2) system call is implemented as
 *int reboot(int magic, int magic2, int flag, void *arg); This test case
 *is written for int reboot(int flag); which is implemented under glibc
 *Therefore this testcase may not work under libc4 and libc5 libraries
 *****************************************************************************/

#include <unistd.h>
#include <sys/reboot.h>
#include <errno.h>
#include <linux/reboot.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

#define INVALID_PARAMETER 100

static void setup();
static void cleanup();
static int setup_test();

char *TCID = "reboot02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static int exp_enos[] = { EINVAL, EPERM, 0 };

static struct test_case_t {
	char *err_desc;		/*error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/*Expected errorvalue string */
} testcase[] = {
	{
	"Invalid flag", EINVAL, "EINVAL"}, {
	"Permission denied", EPERM, "EPERM "}
};

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		for (i = 0; i < TST_TOTAL; i++) {

			Tst_count = 0;
			if (i == 0) {
				TEST(reboot(INVALID_PARAMETER));
			} else {
				/*change the user to nobody */
				if (setup_test() == 0) {
					TEST(reboot(LINUX_REBOOT_CMD_CAD_ON));
					/* Set effective user id back to root */
					if (seteuid(0) == -1) {
						tst_brkm(TBROK, cleanup,
							 "seteuid failed to "
							 "set the effective uid"
							 " to root");
						perror("seteuid");
					}
				} else {
					tst_resm(TWARN, "skipping the test");
					continue;
				}
			}
			/* check return code */
			if ((TEST_RETURN == -1) && (TEST_ERRNO == testcase[i].
						    exp_errno)) {
				tst_resm(TPASS, "reboot(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "reboot(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s and got %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
		}		/*End of TEST LOOPS */
	}

	/*Clean up and exit */
	cleanup();

	tst_exit();
}				/*End of main */

/*
 * setup_test() - This function sets the user as nobdy
 */
int setup_test()
{
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_resm(TWARN, "\"nobody\" user not present. skipping test");
		return -1;
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
		return -1;
	}
	return 0;
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	TEST_PAUSE;

}

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}