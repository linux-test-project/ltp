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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name : symlink03
 *
 * Test Description :
 *   Verify that,
 *   1) symlink(2) returns -1 and sets errno to EACCES if search/write
 *	permission is denied in the directory where the symbolic link is
 *	being created.
 *   2) symlink(2) returns -1 and sets errno to EEXIST if the specified
 *	symbolic link already exists.
 *   3) symlink(2) returns -1 and sets errno to EFAULT if the specified
 *	file or symbolic link points to invalid address.
 *   4) symlink(2) returns -1 and sets errno to ENAMETOOLONG if the
 *	pathname component of symbolic link is too long (ie, > PATH_MAX).
 *   5) symlink(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname of symbolic link is not a directory.
 *   6) symlink(2) returns -1 and sets errno to ENOENT if the component of
 *	symbolic link points to an empty string.
 *
 * Expected Result:
 *  symlink() should fail with return value -1 and set expected errno.
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
 *  symlink03 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
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
 * Restrictions:
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define MODE_RWX        S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE       S_IRUSR | S_IRGRP | S_IROTH
#define DIR_TEMP        "testdir_1"
#define TESTFILE	"testfile"
#define TEST_FILE1      "testdir_1/tfile_1"
#define SYM_FILE1	"testdir_1/sfile_1"
#define TEST_FILE2      "tfile_2"
#define SYM_FILE2	"sfile_2"
#define TEST_FILE3      "tfile_3"
#define SYM_FILE3	"t_file/sfile_3"

char *TCID = "symlink03";
int TST_TOTAL = 1;

int no_setup();
int setup1();			/* setup function to test symlink for EACCES */
int setup2();			/* setup function to test symlink for EEXIST */
int setup3();			/* setup function to test symlink for ENOTDIR */
int longpath_setup();		/* setup function to test chmod for ENAMETOOLONG */

char Longpathname[PATH_MAX + 2];
char High_address_node[64];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *file;
	char *link;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{TEST_FILE1, SYM_FILE1, "No Search permissions to process",
		    EACCES, setup1}, {
	TEST_FILE2, SYM_FILE2, "Specified symlink already exists",
		    EEXIST, setup2}, {
	TESTFILE, NULL, "Invalid address", EFAULT, no_setup}, {
	TESTFILE, Longpathname, "Symlink path too long", ENAMETOOLONG,
		    longpath_setup}, {
	TESTFILE, "", "Symlink Pathname is empty", ENOENT, no_setup}, {
	TEST_FILE3, SYM_FILE3, "Symlink Path contains regular file",
		    ENOTDIR, setup3}, {
	NULL, NULL, NULL, 0, no_setup}
};

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;
	char *test_file;	/* testfile name */
	char *sym_file;		/* symbolic link file name */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */

	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * Invoke setup function to call individual test setup functions
	 * to simulate test conditions.
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			test_file = Test_cases[ind].file;
			sym_file = Test_cases[ind].link;
			test_desc = Test_cases[ind].desc;

			/*
			 * Call symlink(2) to test different test conditions.
			 * verify that it fails with -1 return value and sets
			 * appropriate errno.
			 */
			TEST(symlink(test_file, sym_file));

			if (TEST_RETURN == -1) {
				/*
				 * Perform functional verification if
				 * test executed without (-f) option.
				 */
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS, "symlink() Fails, %s, "
						 "errno=%d", test_desc,
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "symlink() Fails, %s, "
						 "errno=%d, expected errno=%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "symlink() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}

		tst_count++;	/* incr. TEST_LOOP counter */
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Call test specific setup functions.
 */
void setup(void)
{
	int ind;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Switch to nobody user for correct error code collection */
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO | TERRNO, "setuid(%d) failed", ltpuser->pw_uid);

	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		if (!Test_cases[ind].link)
			Test_cases[ind].link = tst_get_bad_addr(cleanup);
		Test_cases[ind].setupfunc();
	}
}

/*
 * int
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *              Hence, this function just returns 0.
 *  This function simply returns 0.
 */
int no_setup(void)
{
	return 0;
}

/*
 * int
 * setup1() - setup function for a test condition for which symlink(2)
 *            returns -1 and sets errno to EACCES.
 *  Create a test directory under temporary directory and create a test file
 *  under this directory with mode "0666" permissions.
 *  Modify the mode permissions on test directory such that process will not
 *  have search permissions on test directory.
 *
 *  The function returns 0.
 */
int setup1(void)
{
	int fd;			/* file handle for testfile */

	SAFE_MKDIR(cleanup, DIR_TEMP, MODE_RWX);

	if ((fd = open(TEST_FILE1, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}
	SAFE_CLOSE(cleanup, fd);

	/* Modify mode permissions on test directory */
	SAFE_CHMOD(cleanup, DIR_TEMP, FILE_MODE);
	return 0;
}

/*
 * int
 * setup2() - EEXIST
 */
int setup2(void)
{
	int fd;			/* file handle for testfile */

	if ((fd = open(TEST_FILE2, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}
	SAFE_CLOSE(cleanup, fd);

	SAFE_SYMLINK(cleanup, TEST_FILE2, SYM_FILE2);
	return 0;
}

/*
 * int
 * longpath_setup() - setup to create a node with a name length exceeding
 *                    the MAX. length of PATH_MAX.
 *   This function retruns 0.
 */
int longpath_setup(void)
{
	int ind;		/* counter variable */

	for (ind = 0; ind <= (PATH_MAX + 1); ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;
}

/*
 * int
 * setup3() - setup function for a test condition for which symlink(2)
 *           returns -1 and sets errno to ENOTDIR.
 *
 *  Create a symlink file under temporary directory so that test tries to
 *  create symlink file "tfile_3" under "t_file" which happens to be
 *  another symlink file.
 */
int setup3(void)
{
	int fd;			/* file handle for testfile */

	/* Creat/open a testfile and close it */
	if ((fd = open("t_file", O_RDWR | O_CREAT, MODE_RWX)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(2) on t_file failed, errno=%d : %s",
			 errno, strerror(errno));
	}
	SAFE_CLOSE(cleanup, fd);
	return 0;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Restore the mode permissions on test directory.
 *  Remove the temporary directory created in the setup.
 */
void cleanup(void)
{

	/* Restore mode permissions on test directory created in setup2() */
	SAFE_CHMOD(NULL, DIR_TEMP, MODE_RWX);

	tst_rmdir();

}
