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
 * Test Name: ftruncate03
 *
 * Test Description:
 *  Verify that,
 *  1) ftruncate(2) returns -1 and sets errno to EINVAL if the specified
 *     file descriptor has an attempt to write, when open for read only.
 *  2) ftruncate(2) returns -1 and sets errno to EBADF if the file descriptor
 *     of the specified file is not valid.
 *
 * Expected Result:
 *  ftruncate() should fail with return value -1 and set expected errno.
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
 *  ftruncate03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEST_FILE1	"test_file1"		/* file under test */
#define TEST_FILE2	"test_file2"		/* file under test */
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define BUF_SIZE	256			/* buffer size */
#define FILE_SIZE	1024			/* test file size */

int no_setup();
int setup1();			/* setup function to test chmod for EBADF */
int setup2();			/* setup function to test chmod for EINVAL */

int fd1;			/* File descriptor for testfile1 */
int fd2;			/* File descriptor for testfile2 */

struct test_case_t {		/* test case struct. to hold ref. test cond's*/
	int fd;
	char *desc;
	int exp_errno;
	int len;
	int (*setupfunc)();
} Test_cases[] = {
	{ 1, "File descriptor not open for writing", EINVAL, -1, setup1 },
	{ 2, "File descriptor is not valid", EBADF, 256, setup2 },
	{ 0, NULL, 0, 0, no_setup }
};

char *TCID="ftruncate03";	/* Test program identifier.    */
int TST_TOTAL=2;		/* Total number of test conditions */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[]={EINVAL, EBADF, 0};

char nobody_uid[] = "nobody";
struct passwd *ltpuser;


void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int
main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */
	int fildes;		/* File descriptor of testfile */
	off_t trunc_len;	/* truncate length */
	int ind;
    
	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/*
	 * Perform global setup for test to call individual test specific
	 * setup functions.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			fildes = Test_cases[ind].fd;
			test_desc = Test_cases[ind].desc;
			trunc_len = Test_cases[ind].len;

			if (fildes == 1) {
				fildes = fd1;
			} else {
				fildes = fd2;
			}

			/* 
			 * Call ftruncate(2) to test different test conditions.
			 * verify that it fails with return code -1 and sets
			 * appropriate errno.
			 */
			TEST(ftruncate(fildes, trunc_len));

			/* check return code of ftruncate(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS, "ftruncate() fails, %s,"
						 " errno=%d", test_desc,
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "ftruncate() fails, %s,"
						 " errno=%d, expected errno:%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "ftruncate() returned %d, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}	/* End of TEST CASE LOOPING. */
	}	/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	/*NOTREACHED*/
}	/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Fill the test buffer with some date used to fill test file(s).
 *  Call individual test specific setup functions.
 */
void 
setup()
{
	int ind;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
        if (geteuid() != 0) {
                tst_brkm(TBROK, tst_exit, "Test must be run as root");
        }
         ltpuser = getpwnam(nobody_uid);
         if (setgid(ltpuser->pw_uid) == -1) {
                tst_resm(TINFO, "setgid failed to "
                         "to set the effective uid to %d",
                         ltpuser->pw_uid);
                perror("setgid");
         }
         if (setuid(ltpuser->pw_uid) == -1) {
                tst_resm(TINFO, "setuid failed to "
                         "to set the effective uid to %d",
                         ltpuser->pw_uid);
                perror("setuid");
         }


	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}	/* End of setup */

/*
 * int
 * setup1() - setup function for a test condition for which ftruncate(2)
 *	      returns -1 and sets errno to EINVAL.
 *  Create a test file and open it for reading only.
 */
int
setup1()
{
	/* Open the testfile in read-only mode */
	if ((fd1 = open(TEST_FILE1, O_RDONLY|O_CREAT, 0644)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDONLY) Failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}
	return 0;
}	/* End setup1() */

/*
 * int
 * setup2() - setup function for a test condition for which ftruncate(2)
 *            returns -1 and sets errno to EBADF.
 *  Create a test file and open it for reading/writing, and fill the
 *  testfile with the contents of test buffer.
 *  Close the test file.
 *
 */
int
setup2()
{
	int c, i, c_total = 0;
	char tst_buff[BUF_SIZE];	/* buffer to hold testfile contents */

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* open a testfile for reading/writing */
	if ((fd2 = open(TEST_FILE2, O_RDWR|O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TEST_FILE2, FILE_MODE, errno, strerror(errno));
	}

	/* Write to the file 1k data from the buffer */
	while (c_total < FILE_SIZE) {
		if ((c = write(fd2, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK, cleanup,
				 "write(2) on %s Failed, errno=%d : %s",
				 TEST_FILE2, errno, strerror(errno));
		} else {
			c_total += c;
		}
	}

	/* Close the testfile after writing data into it */
	if (close(fd2) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}
	return 0;
}	/* End of setup2 */

/*
 * int
 * no_setup() - This function just returns 0.
 */
int
no_setup()
{
	return 0;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the temporary file.
 *  Remove the test directory and testfile created in the setup.
 */
void 
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the testfile after opening it read-only in setup1 */
	if (close(fd1) == -1) {
		tst_brkm(TFAIL, NULL,
			 "close(%s) Failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}	/* End cleanup() */
