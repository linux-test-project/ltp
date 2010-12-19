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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"temp_file"
#define SYM_FILE	"sym_file"
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define EXE_MODE	0777

char *TCID = "access02";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
int fd1, fd2, fd4;		/* file descriptor for testfile(s) */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int setup1();			/* setup() to test access() for R_OK */
int setup2();			/* setup() to test access() for W_OK */
int setup3();			/* setup() to test access() for X_OK */
int setup4();			/* setup() to test access() on symlink file */

struct test_case_t {		/* test case structure */
	char *pathname;
	mode_t a_mode;
	int (*setupfunc) ();
} test_cases[] = {
	/* Read access */
	{ TEST_FILE1, R_OK, setup1 },
	/* Write access */
	{ TEST_FILE2, W_OK, setup2 },
	/* Execute access */
	{ TEST_FILE3, X_OK, setup3 },
	/* Symlink */
	{ SYM_FILE, W_OK, setup4 },
};

void setup();
void cleanup();
int access_verify(int);

int main(int ac, char **av)
{
	int lc;
	int i;
	char *msg;		/* message returned from parse_opts */
	mode_t access_mode;	/* specified access mode for testfile */
	char *file_name;	/* name of the testfile */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			file_name = test_cases[i].pathname;
			access_mode = test_cases[i].a_mode;

			/*
			 * Call access(2) to check the test file
			 * for specified access mode permissions.
			 */
			TEST(access(file_name, access_mode));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO,
					 "access(%s, %#o) failed",
					 file_name, access_mode);
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				access_verify(i);
			} else
				tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();

	tst_exit();
}

void setup()
{
	int i;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TINFO|TERRNO, NULL, "setuid failed");

	TEST_PAUSE;

	tst_tmpdir();

	for (i = 0; i < TST_TOTAL; i++)
		test_cases[i].setupfunc();
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
			 "symlink(%s, %s) failed", TEMP_FILE, SYM_FILE);
	}

	return 0;
}

/*
 * access_verify(i) -
 *
 *	This function verify the accessibility of the
 *	the testfile with the one verified by access().
 */
int access_verify(int i)
{
	char write_buf[] = "abc";
	char read_buf[BUFSIZ];
	int rval;

	rval = 0;

	switch (i) {
	case 0:		/*
			 * The specified file has read access.
			 * Attempt to read some data from the testfile
			 * and if successful, access() behaviour is
			 * correct.
			 */
		if ((rval = read(fd1, &read_buf, sizeof(read_buf))) == -1)
			tst_resm(TFAIL|TERRNO, "read(%s) failed", TEST_FILE1);
		break;
	case 1:		/*
			 * The specified file has write access.
			 * Attempt to write some data to the testfile
			 * and if successful, access() behaviour is correct.
			 */
		if ((rval = write(fd2, write_buf, strlen(write_buf))) == -1)
			tst_resm(TFAIL|TERRNO, "write(%s) failed", TEST_FILE2);
		break;
	case 2:		/*
			 * The specified file has execute access.
			 * Attempt to execute the specified executable
			 * file, if successful, access() behaviour is correct.
			 */
		if ((rval = system("./" TEST_FILE3)) != 0)
			tst_resm(TFAIL, "Fail to execute the %s", TEST_FILE3);
		break;
	case 3:		/*
			 * The file pointed to by symbolic link has
			 * write access.
			 * Attempt to write some data to this temporary file
			 * pointed to by symlink. if successful, access() bahaviour
			 * is correct.
			 */
		if ((rval = write(fd4, write_buf, strlen(write_buf))) == -1)
			tst_resm(TFAIL|TERRNO, "write(%s) failed", TEMP_FILE);
		break;
	default:
		break;
	}

	return rval;
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

}