/*
 *
 *   Copyright (C) Bull S.A. 2001
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
 * Test Name: truncate04
 *
 * Test Description:
 *  Verify that,
 *     truncate(2) returns -1 and sets errno to EISDIR if the named file
 *     is a directory.
 *
 * Expected Result:
 *  truncate() should fail with return value -1 and set expected errno.
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
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *   truncate04 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	05/2002 Jacky Malcles
 *		-Ported
 *
 * Restrictions:
 *  This test should be executed by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define MODES   	S_IRWXU
#define TEST_DIR	"testdir"
#define TRUNC_LEN	256	/* truncation length */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

TCID_DEFINE(truncate04);	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */

char test_desc[] = "File is a directory";
int exp_enos[] = { EISDIR, 0 };
int r_val;
int fd;

void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *file_name;	/* testfile name */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	/*
	 * Perform global setup
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call truncate(2)
		 * verify that it fails with return code -1 and sets
		 * appropriate errno.
		 */
		file_name = TEST_DIR;
		TEST(truncate(file_name, TRUNC_LEN));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EISDIR) {
				tst_resm(TPASS, "truncate() fails, %s, "
					 "errno=%d", test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "truncate() fails, %s, "
					 "errno=%d, expected errno:%d",
					 test_desc, TEST_ERRNO, EISDIR);
			}
		} else {
			tst_resm(TFAIL, "truncate() returned %ld, "
				 "expected -1, errno EISDIR", TEST_RETURN);
		}
	}

	cleanup();
	tst_exit();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test directory under temporary directory and open it
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/*
	 * Pause if that option was specified
	 */
	TEST_PAUSE;

	tst_tmpdir();

	/*
	 * create a new directory and open it
	 */

	if ((r_val = mkdir(TEST_DIR, MODES)) == -1) {
		tst_brkm(TBROK, cleanup, "%s - mkdir() in main() "
			 "failed", TCID);
	}

	if ((fd = open(TEST_DIR, O_RDONLY)) == -1) {
		tst_brkm(TBROK, cleanup, "open of directory failed");
	}

}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	if (close(fd) < 0) {
		tst_brkm(TBROK, cleanup, "close failed: errno = %d", errno);
	}

	tst_rmdir();

}