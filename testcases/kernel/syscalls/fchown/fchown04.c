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
 * Test Name: fchown04
 *
 * Test Description:
 *   Verify that,
 *   1) fchown(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) fchown(2) returns -1 and sets errno to EBADF if the file descriptor
 *	of the specified file is not valid.
 *
 * Expected Result:
 *  fchown() should fail with return value -1 and set expected errno.
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
 *  fchown04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be executed by 'non-super-user' only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX	S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"tfile_2"

int no_setup();
int setup1();			/* setup function to test chmod for EPERM */
int setup2();			/* setup function to test chmod for EBADF */

int fd1;			/* File descriptor for testfile1 */
int fd2;			/* File descriptor for testfile2 */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int fd;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	1, "Process is not owner/root", EPERM, setup1}, {
	2, "File descriptor is not valid", EBADF, setup2}, {
	0, NULL, 0, no_setup}
};

char test_home[PATH_MAX];	/* variable to hold TESTHOME env */
char *TCID = "fchown04";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EPERM, EBADF, 0 };

char bin_uid[] = "bin";
struct passwd *ltpuser;

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */
	int fd;			/* test file descriptor */
	int ind;		/* counter to test different test conditions */
	uid_t User_id;		/* Effective user id of a test process */
	gid_t Group_id;		/* Effective group id of a test process */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/*
	 * Invoke setup function to call individual test setup functions
	 * to simulate test conditions.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Set uid/gid values to that of test process */
	User_id = geteuid();
	Group_id = getegid();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			fd = Test_cases[ind].fd;
			test_desc = Test_cases[ind].desc;

			if (fd == 1) {
				fd = fd1;
			} else {
				fd = fd2;
			}

			/*
			 * Call fchown(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(fchown(fd, User_id, Group_id));

			/* Check return code from fchown(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS,
						 "fchown() fails, %s, errno:%d",
						 test_desc, TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "fchown() fails, %s, "
						 "errno:%d, expected errno:%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "fchown() returned %ld, expected"
					 " -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */

	}			/* End for TEST_LOOPING */

	/*
	 * Invoke cleanup() to delete the test directory/file(s) created
	 * in the setup().
	 */
	cleanup();

	 /*NOTREACHED*/ return (0);
}				/* End main */

/*
 * setup(void) - performs all ONE TIME setup for this test.
 *
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 *	Invoke individual test setup functions according to the order
 *	set in struct. definition.
 */
void setup()
{
	int ind;		/* counter for setup functions */

	ltpuser = getpwnam(bin_uid);

	/* Capture unexpected signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

/*
 * changed by prashant yendigeri because the temp directory creating and
 * deleting are with different uids, so this test case will fail in most of
 * the scenario.
 */
	/* Make a temp dir and cd to it */
	/* tst_tmpdir(); */

	/* Switch to bin user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, cleanup, "Test must be run as root");
	}

	/* to let chown(TESTDIR, -1, getgid()) in tst_tmpdir() run
	 * successfully even if getgid() is not in group_info of
	 * the current process's task_struct till now
	 */

	initgroups("root", getgid());

	if (setegid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setegid failed to "
			 "to set the effective gid to %d", ltpuser->pw_uid);
		perror("setegid");
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
	}
//prashant yendigeri added the below line
	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}				/* End setup() */

/*
 * setup1() - setup function for a test condition for which fchown(2)
 *	      returns -1 and sets errno to EPERM.
 *
 *  Create a test file under temporary directory.
 *  Get the current working directory of the process and invoke setuid
 *  to root program to change the ownership of testfile to that of
 *  "ltpuser2" user.
 */
int setup1()
{
	int old_uid;
	struct passwd *nobody;

	/* Create a testfile under temporary directory */
	fd1 = open(TEST_FILE1, O_RDWR | O_CREAT, 0666);
	if (fd1 == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}

	old_uid = geteuid();
	seteuid(0);

	nobody = getpwnam("nobody");
	if (!nobody)
		tst_brkm(TBROK, cleanup, "Couldn't find user nobody: %s",
						strerror(errno));

	if (fchown(fd1, nobody->pw_uid, nobody->pw_gid) < 0)
		tst_brkm(TBROK, cleanup, "Fail to modify %s ownership(s)! %s",
					TEST_FILE1, strerror(errno));

	seteuid(old_uid);

	return 0;
}

/*
 * setup2() - setup function for a test condition for which fchown(2)
 *	      returns -1 and sets errno to EBADF.
 *
 *  Create a testfile under temporary directory and close it such that
 *  fchown(2) attempts to modify the file which is already closed.
 */
int setup2()
{
	/* Create a testfile under temporary directory */
	if ((fd2 = open(TEST_FILE2, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}

	/* Close the testfile opened above */
	if (close(fd2) == -1) {
		tst_brkm(TBROK, cleanup, "close(%s) Failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}
	return 0;
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
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *	Print test timing stats and errno log if test executed with options.
 *	Close the testfile if still opened.
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

	if (close(fd1) == -1) {
		tst_resm(TBROK, "close(%s) Failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}

	/* Remove files and temporary directory created */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
