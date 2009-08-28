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
 * Test Name: access05
 *
 * Test Description:
 *  Verify that,
 *   1. access() fails with -1 return value and sets errno to EACCES
 *      if the permission bits of the file mode do not permit the
 *	 requested (Read/Write/Execute) access.
 *   2. access() fails with -1 return value and sets errno to EINVAL
 *	if the specified access mode argument is invalid.
 *   3. access() fails with -1 return value and sets errno to EFAULT
 *	if the pathname points outside allocate address space for the
 *	process.
 *   4. access() fails with -1 return value and sets errno to ENOENT
 *	if the specified file doesn't exist (or pathname is NULL).
 *   5. access() fails with -1 return value and sets errno to ENAMETOOLONG
 *      if the pathname size is > PATH_MAX characters.
 *
 * Expected Result:
 *  access() should fail with return value -1 and set expected errno.
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
 *  access05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define INV_OK		-1
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define TEST_FILE4	"test_file4"

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

int no_setup();
int setup1();			/* setup() to test access() for EACCES */
int setup2();			/* setup() to test access() for EACCES */
int setup3();			/* setup() to test access() for EACCES */
int setup4();			/* setup() to test access() for EINVAL */
int longpath_setup();		/* setup function to test access() for ENAMETOOLONG */

#if !defined(UCLINUX)
char *get_high_address();	/* function from ltp-lib        */
char High_address_node[64];
#endif

char Longpathname[PATH_MAX + 2];

struct test_case_t {		/* test case structure */
	char *pathname;
	int a_mode;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	TEST_FILE1, R_OK, "Read Access denied on file", EACCES, setup1}, {
	TEST_FILE2, W_OK, "Write Access denied on file", EACCES, setup2}, {
	TEST_FILE3, X_OK, "Execute Access denied on file", EACCES, setup3},
	{
	TEST_FILE4, INV_OK, "Access mode invalid", EINVAL, setup4},
#if !defined(UCLINUX)
	{
	(char *)-1, R_OK, "Negative address", EFAULT, no_setup}, {
	High_address_node, R_OK, "Address beyond address space", EFAULT,
		    no_setup},
#endif
	{
	"", W_OK, "Pathname is empty", ENOENT, no_setup}, {
	Longpathname, R_OK, "Pathname too long", ENAMETOOLONG, longpath_setup},
	{
	NULL, 0, NULL, 0, no_setup}
};

char *TCID = "access05";	/* Test program identifier.    */
int TST_TOTAL = 8;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EACCES, EFAULT, EINVAL, ENOENT, ENAMETOOLONG, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

char *bad_addr = 0;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *file_name;	/* name of the testfile */
	char *test_desc;	/* test specific message */
	int access_mode;	/* specified access mode for testfile */
	int ind;		/* counter for testcase looping */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			file_name = Test_cases[ind].pathname;
			access_mode = Test_cases[ind].a_mode;
			test_desc = Test_cases[ind].desc;

#if !defined(UCLINUX)
			if (file_name == High_address_node) {
				file_name = get_high_address();
			}
#endif

			/*
			 * Call access(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(access(file_name, access_mode));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "access() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			/*
			 * Call a function to verify whether
			 * the specified file has specified
			 * access mode.
			 */
			if (TEST_ERRNO == Test_cases[ind].exp_errno) {
				tst_resm(TPASS|TTERRNO, "access() fails, %s",
					 test_desc);
			} else {
				tst_resm(TFAIL|TTERRNO, "access() fails %s (expected errno %d)",
					 test_desc, Test_cases[ind].exp_errno);
			}
		}		/* Test Case Looping */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Create a temporary directory and change directory to it.
 *  Call individual test specific setup functions.
 */
void setup()
{
	int ind;		/* counter for testsetup functions */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO|TERRNO, "setuid(%d) failed", ltpuser->pw_uid);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");
	}
	Test_cases[5].pathname = bad_addr;
#endif

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * no_setup() - some test conditions do not need any setup.
 *		Hence, this function simply returns 0.
 */
int no_setup()
{
	return 0;
}

int setup_file(const char *file, mode_t perms)
{
	int fd;		/* file handle for testfile */

	/* Creat a test file under above directory created */
	fd = open(file, O_RDWR | O_CREAT, FILE_MODE);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 file, FILE_MODE);

	/* Close the testfile created above */
	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", file);

	/* Change mode permissions on testfile */
	if (chmod(file, perms) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s, %#o) failed",
			 file, perms);

	return 0;
}

/*
 * setup1() - Setup function to test access() for return value -1
 *	      and errno EACCES when read access denied for specified
 *	      testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny read access permissions on testfile.
 *   This function returns 0.
 */
int setup1()
{
	return setup_file(TEST_FILE1, 0333);
}

/*
 * setup2() - Setup function to test access() for return value -1 and
 *	      errno EACCES when write access denied on testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny write access permissions on testfile.
 *   This function returns 0.
 */
int setup2()
{
	return setup_file(TEST_FILE2, 0555);
}

/*
 * setup3() - Setup function to test access() for return value -1 and
 *	      errno EACCES when execute access denied on testfile.
 *
 *   Creat/open a testfile and close it.
 *   Deny search access permissions on testfile.
 *   This function returns 0.
 */
int setup3()
{
	return setup_file(TEST_FILE3, 0666);
}

/*
 * setup4() - Setup function to test access() for return value -1
 *	      and errno EINVAL when specified access mode argument is
 *	      invalid.
 *
 *   Creat/open a testfile and close it.
 *   This function returns 0.
 */
int setup4()
{
	return setup_file(TEST_FILE4, FILE_MODE);
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *		      the MAX. length of PATH_MAX.
 */
int longpath_setup()
{
	int ind;

	for (ind = 0; ind <= (PATH_MAX + 1); ind++) {
		Longpathname[ind] = 'a';
	}

	return 0;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Delete the test directory/file and temporary directory
	 * created in the setup.
	 */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
