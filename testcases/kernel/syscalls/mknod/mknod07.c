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
 * Test Name: mknod07
 *
 * Test Description:
 * Verify that,
 *   1) mknod(2) returns -1 and sets errno to EPERM if the process id of
 *	the caller is not super-user.
 *   2) mknod(2) returns -1 and sets errno to EACCES if parent directory
 *	does not allow  write  permission  to  the process.
 *
 * Expected Result:
 *  mknod() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  mknod07 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by non-super-user only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX	S_IFIFO | S_IRWXU | S_IRWXG | S_IRWXO
#define NEWMODE		S_IFIFO | S_IRUSR | S_IRGRP | S_IROTH
#define SOCKET_MODE	S_IFSOCK| S_IRWXU | S_IRWXG | S_IRWXO
#define DIR_TEMP	"testdir_1"

int no_setup();
int setup2();			/* setup function to test mknod for EACCES */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	char *desc;
	int mode;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	"tnode_1", "Process is not root/super-user", SOCKET_MODE,
		    EACCES, no_setup}, {
	"tnode_2", "No Write permissions to process", NEWMODE, EACCES, setup2},
	{
	NULL, NULL, 0, 0, no_setup}
};

char *TCID = "mknod07";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EPERM, EACCES, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* setup function for the tests */
void cleanup();			/* cleanup function for the tests */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *node_name;	/* ptr. for node name created */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */
	int mode;		/* creation mode for the node created */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/*
	 * Invoke setup function to call individual test setup functions
	 * for the test which run as root/super-user.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			node_name = Test_cases[ind].pathname;
			mode = Test_cases[ind].mode;
			test_desc = Test_cases[ind].desc;

			/*
			 * Call mknod(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(mknod(node_name, mode, 0));

			/* Check return code from mknod(2) */
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "mknod() returned %ld, expected "
					 "-1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == Test_cases[ind].exp_errno) {
				tst_resm(TPASS, "mknod() fails, %s, errno:%d",
					 test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "mknod() fails, %s, errno:%d, "
					 "expected errno:%d", test_desc,
					 TEST_ERRNO, Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */

	}			/* End for TEST_LOOPING */

	/*
	 * Invoke cleanup() to delete the test directories created
	 * in the setup().
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup(void) - performs all ONE TIME setup for this test.
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory used to hold test directories and nodes
 *	created and change the directory to it.
 *	Create a test directory with ownership of test process id and
 *	change the directory to it.
 *	Invoke individual test setup functions according to the order
 *	set in struct. definition.
 */
void setup()
{
	int ind;		/* counter for setup functions */

	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, NULL, "setuid failed to "
			 "to set the effective uid to %d: %s",
			 ltpuser->pw_uid, strerror(errno));
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/*
	 * Create a test directory under temporary directory with the
	 * specified mode permissions, with uid/gid set to that of guest
	 * user and the test process.
	 */
	if (mkdir(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) of %s failed", DIR_TEMP);
		tst_exit();
	}

	if (chmod(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", DIR_TEMP);
	}

	/* Change directory to DIR_TEMP */
	if (chdir(DIR_TEMP) < 0) {
		tst_brkm(TBROK, cleanup,
			 "Unable to change to %s directory", DIR_TEMP);
	}

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *              Hence, this function just returns 0.
 */
int no_setup()
{
	return 0;
}

/*
 * setup2() - setup function for a test condition for which mknod(2)
 *	      returns -1 and sets errno to EACCES.
 *  This setup changes mode permission bits on test directory such that process
 *  will not have write permission to create node(s) using mknod(2).
 *  The function returns 0.
 */
int setup2()
{
	/* Modify mode permissions on test directory */
	if (chmod(".", NEWMODE) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", DIR_TEMP);
	}
	return 0;
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	if (seteuid(0) == -1)
		tst_resm(TBROK, "Couldn't get root back: %s", strerror(errno));

	/* Remove files and temporary directory created */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
