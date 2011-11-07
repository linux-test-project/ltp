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
 * Test Name : readlink03
 *
 * Test Description :
 *   Verify that,
 *   1) readlink(2) returns -1 and sets errno to EACCES if search/write
 *	permission is denied in the directory where the symbolic link
 *	resides.
 *   2) readlink(2) returns -1 and sets errno to EINVAL if the buffer size
 *	is not positive.
 *   3) readlink(2) returns -1 and sets errno to EINVAL if the specified
 *	file is not a symbolic link file.
 *   4) readlink(2) returns -1 and sets errno to ENAMETOOLONG if the
 *	pathname component of symbolic link is too long (ie, > PATH_MAX).
 *   5) readlink(2) returns -1 and sets errno to ENOENT if the component of
 *	symbolic link points to an empty string.
 *
 * Expected Result:
 *  readlink() should fail with return value -1 and set expected errno.
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
 *  readlink03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX        S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE       S_IRUSR | S_IRGRP | S_IROTH
#define DIR_TEMP        "testdir_1"
#define TESTFILE	"testfile"
#define TEST_FILE1      "testdir_1/tfile_1"
#define SYM_FILE1	"testdir_1/sfile_1"
#define TEST_FILE2      "tfile_2"
#define SYM_FILE2	"sfile_2"
#define MAX_SIZE	256

char *TCID = "readlink03";	/* Test program identifier.    */
int TST_TOTAL = 5;		/* Total number of test cases. */
int exp_enos[] = { EACCES, EINVAL, ENAMETOOLONG, ENOENT, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int no_setup();
int setup1();			/* setup function to test symlink for EACCES */
int setup2();			/* setup function to test symlink for EEXIST */
int lpath_setup();		/* setup function to test chmod for ENAMETOOLONG */

char Longpathname[PATH_MAX + 2];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *link;
	char *desc;
	int exp_errno;
	size_t buf_siz;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	SYM_FILE1, "No Search permissions to process", EACCES, 1, setup1},
	    /* Don't test with bufsize -1, since this cause a fortify-check-fail when
	       using glibc and -D_FORITY_SOURCE=2

	       Discussion: http://lkml.org/lkml/2008/10/23/229
	       Conclusion: Only test with 0 as non-positive bufsize.

	       { SYM_FILE2, "Buffer size is not positive", EINVAL, -1, setup2 },
	     */
	{
	SYM_FILE2, "Buffer size is not positive", EINVAL, 0, setup2}, {
	TEST_FILE2, "File is not symbolic link", EINVAL, 1, no_setup}, {
	Longpathname, "Symlink path too long", ENAMETOOLONG, 1, lpath_setup},
	{
	"", "Symlink Pathname is empty", ENOENT, 1, no_setup}, {
	NULL, NULL, 0, 0, no_setup}
};

void setup();			/* Setup function for the test */
void cleanup();			/* Cleanup function for the test */

int main(int ac, char **av)
{
	char buffer[MAX_SIZE];	/* temporary buffer to hold symlink contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *sym_file;		/* symbolic link file name */
	char *test_desc;	/* test specific error message */
	int i;			/* counter to test different test conditions */
	size_t buf_size;	/* size of buffer for readlink */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/*
	 * Invoke setup function to call individual test setup functions
	 * to simulate test conditions.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; Test_cases[i].desc != NULL; i++) {
			sym_file = Test_cases[i].link;
			test_desc = Test_cases[i].desc;
			buf_size = Test_cases[i].buf_siz;

			if (buf_size == 1) {
				buf_size = sizeof(buffer);
			}

			/*
			 * Call readlink(2) to test different test conditions.
			 * verify that it fails with -1 return value and sets
			 * appropriate errno.
			 */
			TEST(readlink(sym_file, buffer, buf_size));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "readlink() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[i].exp_errno);
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == Test_cases[i].exp_errno) {
				tst_resm(TPASS, "readlink(), %s, returned "
					 "errno %d", test_desc, TEST_ERRNO);
				tst_resm(TPASS, "readlink(), %s, returned "
					 "errno %d", test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "readlink() failed, %s, "
					 "errno=%d, expected errno=%d",
					 test_desc, TEST_ERRNO,
					 Test_cases[i].exp_errno);
				if ((strncmp(test_desc, "Symlink Pathname is empty", 25) == 0) &&
				     TEST_ERRNO == EINVAL)
					tst_resm(TWARN, "It may be a Kernel Bug, see the patch:"
						 "http://git.kernel.org/linus/1fa1e7f6");
			}
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Create a temporary directory and change directory to it.
 *  Call test specific setup functions.
 */
void setup()
{
	int i;

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwname(nobody_uid) failed ");
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* call individual setup functions */
	for (i = 0; Test_cases[i].desc != NULL; i++) {
		Test_cases[i].setupfunc();
	}
}

/*
 * no_setup() - Some test conditions for readlink(2) do not any setup.
 */
int no_setup()
{
	return 0;
}

/*
 * setup1() - setup function for a test condition for which readlink(2)
 *            returns -1 and sets errno to EACCES.
 *
 *  Create a test directory under temporary directory and create a test file
 *  under this directory with mode "0666" permissions.
 *  Create a symlink of testfile under test directory.
 *  Modify the mode permissions on test directory such that process will not
 *  have search permissions on test directory.
 */
int setup1()
{
	int fd;			/* file handle for testfile */

	if (mkdir(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) of %s failed", DIR_TEMP);
	}

	if ((fd = open(TEST_FILE1, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close(%s) failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}

	/* Creat a symbolic link of testfile under test directory */
	if (symlink(TEST_FILE1, SYM_FILE1) < 0) {
		tst_brkm(TBROK, cleanup, "symlink of %s failed", TEST_FILE1);
	}

	/* Modify mode permissions on test directory */
	if (chmod(DIR_TEMP, FILE_MODE) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", DIR_TEMP);
	}
	return 0;
}

/*
 * setup2() -  setup function for a test condition for which readlink(2)
 *	       returns -1 and sets errno to EINVAL.
 *
 *	Create a testfile under temporary directory and create a symlink
 *	file of it.
 */
int setup2()
{
	int fd;			/* file handle for testfile */

	/* Creat a testfile and close it */
	if ((fd = open(TEST_FILE2, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close(%s) failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}

	/* Creat a symlink of testfile created above */
	if (symlink(TEST_FILE2, SYM_FILE2) < 0) {
		tst_brkm(TBROK, cleanup, "symlink() failed to create %s in "
			 "setup2, error=%d", SYM_FILE2, errno);
	}
	return 0;
}

/*
 * lpath_setup() - setup to create a node with a name length exceeding
 *		   the MAX. length of PATH_MAX.
 */
int lpath_setup()
{
	int i;			/* counter variable */

	for (i = 0; i <= (PATH_MAX + 1); i++) {
		Longpathname[i] = 'a';
	}
	return 0;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Restore the mode permissions on test directory.
 *  Remove the temporary directory created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Restore mode permissions on test directory created in setup2() */
	if (chmod(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, NULL, "chmod(2) of %s failed", DIR_TEMP);
	}

	tst_rmdir();

}
