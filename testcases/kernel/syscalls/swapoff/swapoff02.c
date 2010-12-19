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
 *    TEST IDENTIFIER	: swapoff02
 *
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test checking for basic error conditions
 *    				 for swapoff(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case checks whether swapoff(2) system call  returns
 *	1. EINVAL when the path does not exist
 *	2. ENOENT when the path exists but is invalid
 *	3. EPERM when user is not a superuser
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	 1.  For testing error on invalid user, change the effective uid
 * 	$
 * 	Test:
 *	  Loop if the proper options are given.
 *	  Execute system call.
 *	  Check return code, if system call fails with errno == expected errno
 *		Issue syscall passed with expected errno
 *	  Otherwise,
 *	  Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	  Do cleanup for the test.
 * 	 $
 * USAGE:  <for command-line>
 *  swapoff02 [-c n] [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where
 *  	-c n : Run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *Incompatible with kernel versions below 2.1.35.
 *
 *CHANGES:
 * 2005/01/01  Add extra check to stop test if insufficient disk space in dir
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 * 2005/01/01  Add extra check to stop test if swap file is on tmpfs
 *             -Ricky Ng-Adam (rngadam@yahoo.com)
 *****************************************************************************/

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include "test.h"
#include "usctest.h"
#include <stdlib.h>
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"

static void setup();
static void cleanup();
static int setup01();
static int cleanup01();
static int setup02();

char *TCID = "swapoff02";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test cases. */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;
int need_swapfile_cleanup = 0;	/* attempt to swapoff in cleanup */

static int exp_enos[] = { EPERM, EINVAL, ENOENT, 0 };

static struct test_case_t {
	char *err_desc;		/* error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/* Expected errorvalue string */
	char *path;		/* path for swapon */
	int (*setupfunc) ();	/* Test setup function */
	int (*cleanfunc) ();	/* Test cleanup function */
} testcase[] = {
	{
	"path does not exist", ENOENT, "ENOENT", "./abcd", NULL, NULL}, {
	"Invalid path", EINVAL, "EINVAL ", "./nofile", setup02, NULL}, {
	"Permission denied", EPERM, "EPERM ", "./swapfile01",
		    setup01, cleanup01}
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

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (testcase[i].setupfunc &&
			    testcase[i].setupfunc() == -1) {
				tst_resm(TWARN, "Failed to setup test %d."
					 " Skipping test", i);
				continue;
			} else {
				TEST(syscall(__NR_swapoff, testcase[i].path));
			}

			if (testcase[i].cleanfunc &&
			    testcase[i].cleanfunc() == -1) {
				tst_brkm(TBROK, cleanup, "cleanup failed,"
					 " quitting the test");
			}

			/* check return code */
			if ((TEST_RETURN == -1) && (TEST_ERRNO == testcase[i].
						    exp_errno)) {
				tst_resm(TPASS, "swapoff(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);

			} else {
				tst_resm(TFAIL, "swapoff(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s and got %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);

				if ((TEST_RETURN == 0) && (i == 2)) {
					if (syscall(__NR_swapon, "./swapfile01", 0) != 0) {
						tst_brkm(TBROK, cleanup,
							 " Failed to turn on"
							 " swap file");
					}
				}
			}

			TEST_ERROR_LOG(TEST_ERRNO);
		}		/*End of TEST LOOPS */
	}

	/*Clean up and exit */
	cleanup();

	tst_exit();
}				/*End of main */

/*
 * setup01() - This function sets the user as nobody
 */
int setup01()
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

	return 0;		/* user switched to nobody */
}

/*
 * cleanup01() - switch back to user root
 */
int cleanup01()
{
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, cleanup, "seteuid failed to set uid to root");
		perror("seteuid");
		return -1;
	}

	return 0;
}

int setup02()
{
	int fd;
	fd = creat("nofile", S_IRWXU);
	if (fd == -1)
		tst_resm(TWARN, "Failed to create temporary file");
	return 0;
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	if (!tst_cwd_has_free(65536)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	/*create file */
	if (system
	    ("dd if=/dev/zero of=swapfile01 bs=1024  count=65536 > tmpfile"
	     " 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to create file for swap");
	}

	/* make above file a swap file */
	if (system("mkswap ./swapfile01 > tmpfile 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
	}

	if (syscall(__NR_swapon, "./swapfile01", 0) != 0) {
		tst_brkm(TBROK, cleanup, "Failed to turn on the swap file."
			 " skipping  the test iteration");
	}

	need_swapfile_cleanup = 1;

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

	if (need_swapfile_cleanup && (syscall(__NR_swapoff, "./swapfile01") != 0)) {
		tst_resm(TWARN, " Failed to turn off swap file. System reboot"
			 " after execution of LTP test suite is"
			 " recommended.");
	}

	tst_rmdir();

}