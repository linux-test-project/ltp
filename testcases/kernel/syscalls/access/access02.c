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
 * Test Name: access02
 *
 * Test Description:
 *  Verify that access() succeeds to check the read/write/execute permissions
 *  on a file if the mode argument passed was R_OK/W_OK/X_OK.
 *
 *  Also verify that, access() succeeds to test the accessibility of the file
 *  referred to by symbolic link if the pathname points to a symbolic link.
 *
 * Expected Result:
 *  access() should return 0 value and the specified file should have
 *  respective permissions set on it.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  access02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"temp_file"
#define SYM_FILE	"sym_file"
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define EXE_MODE	0777

char *TCID = "access02";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int fd1, fd2, fd4;		/* file descriptor for testfile(s) */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int setup1();			/* setup() to test access() for R_OK */
int setup2();			/* setup() to test access() for W_OK */
int setup3();			/* setup() to test access() for X_OK */
int setup4();			/* setup() to test access() on symlink file */

struct test_case_t {		/* test case structure */
	char *pathname;
	int a_mode;
	char *desc;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	TEST_FILE1, R_OK, "Read Access (R_OK)", setup1}, {
	TEST_FILE2, W_OK, "Write Access (W_OK)", setup2}, {
	TEST_FILE3, X_OK, "Execute Access (X_OK)", setup3}, {
	SYM_FILE, W_OK, "Symlink file", setup4}, {
	NULL, 0, NULL, 0}
};

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
int Access_verify(int, int);	/*
				 * Function to verify the actual accessibility
				 * of test file(s).
				 */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int ind;		/* counter for testcase looping */
	char *msg;		/* message returned from parse_opts */
	int fflag;		/* functionality flag variable */
	int access_mode;	/* specified access mode for testfile */
	char *file_name;	/* name of the testfile */
	char *test_desc;	/* test specific message */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			file_name = Test_cases[ind].pathname;
			access_mode = Test_cases[ind].a_mode;
			test_desc = Test_cases[ind].desc;

			/*
			 * Call access(2) to check the test file
			 * for specified access mode permissions.
			 */
			TEST(access(file_name, access_mode));

			/* check return code of access(2) */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO,
					 "access(%s, %#o) failed",
					 file_name, access_mode);
				continue;
			}

			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/* Set the functionality flag */
				fflag = 1;

				/*
				 * Call a function to verify whether
				 * the specified file has specified
				 * access mode.
				 */
				fflag = Access_verify(ind, fflag);
				if (fflag) {
					tst_resm(TPASS, "access(%s): %s test",
						 file_name, test_desc);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
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
	tst_sig(FORK, DEF_HANDLER, cleanup);

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

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * setup1() - Setup function to test the functionality of access() for
 *	      the access mode argument R_OK.
 *
 *   Creat/open a testfile and write some data into it.
 *   This function returns 0.
 */
int setup1()
{
	char write_buf[] = "abc";

	/* Creat a test file under above directory created */
	if ((fd1 = open(TEST_FILE1, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE1, FILE_MODE);
	}

	/* write some data into testfile */
	if (write(fd1, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "write(%s) failed in setup1",
			 TEST_FILE1);
	}

	return 0;
}

/*
 * setup2() - Setup function to test the functionality of access() for
 *	      the access mode argument W_OK.
 *
 *   Creat/open a testfile for writing under temporary directory.
 *   This function returns 0.
 */
int setup2()
{
	/* Creat a test file under temporary directory */
	if ((fd2 = open(TEST_FILE2, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE2, FILE_MODE);
	}

	return 0;
}

/*
 * setup3() - Setup function to test the functionality of access() for
 *	      the access mode argument X_OK.
 *
 *   Creat/open a testfile and provide execute permissions to it.
 *   This function returns 0.
 */
int setup3()
{
	int fd3;		/* File handle for test file */
#ifdef UCLINUX
	char exechead[] = "#!/bin/sh\n";
#endif

	/* Creat a test file under temporary directory */
	if ((fd3 = open(TEST_FILE3, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE3, FILE_MODE);
	}
#ifdef UCLINUX
	if (write(fd3, exechead, sizeof(exechead)) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "write(%s) failed",
			 TEST_FILE3);
	}
#endif

	/* Close the test file created above */
	if (close(fd3) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed",
			 TEST_FILE3);
	}

	/* Set execute permission bits on the test file. */
	if (chmod(TEST_FILE3, EXE_MODE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s, %#o) failed",
			 TEST_FILE3, EXE_MODE);
	}

	return 0;
}

/*
 * setup4() - Setup function to test the functionality of access() for
 *	      symbolic link file.
 *
 *   Creat/open a temporary file and close it.
 *   Creat a symbolic link of temporary file.
 *   This function returns 0.
 */
int setup4()
{
	/* Creat a temporary  file under temporary directory */
	if ((fd4 = open(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEMP_FILE, FILE_MODE);
	}

	/* Creat a symbolic link for temporary file */
	if (symlink(TEMP_FILE, SYM_FILE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "symlink(%s, %s) failed",
			 TEMP_FILE, SYM_FILE);
	}

	return 0;
}

/*
 * Access_verify(ind, fflag) -
 *
 *	This function verify the Accessibility of the
 *	the testfile with the one verified by access().
 *	This function sets the fflag variable value to 0 if
 *	any verification is false.
 *	Otherwise, returns the already set fflag value.
 */
int Access_verify(int ind, int fflag)
{
	char write_buf[] = "abc";
	char read_buf[BUFSIZ];

	switch (ind) {
	case 0:		/*
			 * The specified file has read access.
			 * Attempt to read some data from the testfile
			 * and if successful, access() behaviour is
			 * correct.
			 */
		if (read(fd1, &read_buf, sizeof(read_buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "read(%s) failed", TEST_FILE1);
			fflag = 0;
		}
		break;
	case 1:		/*
			 * The specified file has write access.
			 * Attempt to write some data to the testfile
			 * and if successful, access() behaviour is correct.
			 */
		if (write(fd2, write_buf, strlen(write_buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "write(%s) failed", TEST_FILE2);
			fflag = 0;
		}
		break;
	case 2:		/*
			 * The specified file has execute access.
			 * Attempt to execute the specified executable
			 * file, if successful, access() behaviour is correct.
			 */
		if (system("./" TEST_FILE3) != 0) {
			tst_resm(TFAIL, "Fail to execute the %s", TEST_FILE3);
			fflag = 0;
		}
		break;
	case 3:		/*
			 * The file pointed to by symbolic link has
			 * write access.
			 * Attempt to write some data to this temporary file
			 * pointed to by symlink. if successful, access() bahaviour
			 * is correct.
			 */
		if (write(fd4, write_buf, strlen(write_buf)) < 0) {
			tst_resm(TFAIL|TERRNO, "write(%s) failed", TEMP_FILE);
			fflag = 0;
		}
		break;
	default:
		break;
	}

	return (fflag);
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

	/* Close the testfile(s) created in the setup()s */
	if (close(fd1) == -1)
		tst_brkm(TFAIL|TERRNO, NULL, "close(%s) failed", TEST_FILE1);
	if (close(fd2) == -1)
		tst_brkm(TFAIL|TERRNO, NULL, "close(%s) failed", TEST_FILE2);
	if (close(fd4) == -1)
		tst_brkm(TFAIL|TERRNO, NULL, "close(%s) failed", TEMP_FILE);

	/*
	 * Delete the test directory/file and temporary directory
	 * created in the setup.
	 */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
