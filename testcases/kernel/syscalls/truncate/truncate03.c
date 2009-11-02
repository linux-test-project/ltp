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
 * Test Name: truncate03
 *
 * Test Description:
 *  Verify that,
 *  1) truncate(2) returns -1 and sets errno to EACCES if search/write
 *     permission denied for the process on the component of the path prefix
 *     or named file.
 *  2) truncate(2) returns -1 and sets errno to ENOTDIR if the component of
 *     the path prefix is not a directory.
 *  3) truncate(2) returns -1 and sets errno to EFAULT if pathname points
 *     outside user's accessible address space.
 *  4) truncate(2) returns -1 and sets errno to ENAMETOOLONG if the component
 *     of a pathname exceeded 255 characters or entire pathname exceeds 1023
 *     characters.
 *  5) truncate(2) returns -1 and sets errno to ENOENT if the named file
 *     does not exist.
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
 *   truncate03 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
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
 *  This test should be executed by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEST_FILE1	"testfile"	/* file under test */
#define TEST_FILE2	"t_file/testfile"	/* file under test */
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define NEW_MODE	S_IRUSR | S_IRGRP | S_IROTH
#define BUF_SIZE	256	/* buffer size */
#define FILE_SIZE	1024	/* test file size */
#define TRUNC_LEN	256	/* truncation length */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int no_setup();
int setup1();			/* setup function to test chmod for EACCES */
int setup2();			/* setup function to test chmod for ENOTDIR */
int longpath_setup();		/* setup function to test chmod for ENAMETOOLONG */

TCID_DEFINE(truncate03);	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EACCES, ENOTDIR, EFAULT, ENAMETOOLONG, ENOENT, 0 };

char *bad_addr = 0;

char Longpathname[PATH_MAX + 2];
char High_address_node[64];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	TEST_FILE1, "No Search permissions to process", EACCES, setup1}, {
	TEST_FILE2, "Path contains regular file", ENOTDIR, setup2},
#if !defined(UCLINUX)
	{
	High_address_node, "Address beyond address space", EFAULT, no_setup},
	{
	(char *)-1, "Negative address", EFAULT, no_setup},
#endif
	{
	Longpathname, "Pathname too long", ENAMETOOLONG, longpath_setup}, {
	"", "Pathname is empty", ENOENT, no_setup}, {
	NULL, NULL, 0, no_setup}
};
int TST_TOTAL = sizeof(Test_cases) / sizeof(*Test_cases);

void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *file_name;	/* testfile name */
	char *test_desc;	/* test specific error message */
	int ind;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
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
			file_name = Test_cases[ind].pathname;
			test_desc = Test_cases[ind].desc;

#if !defined(UCLINUX)
			if (file_name == High_address_node) {
				file_name = (char *)get_high_address();
			}
#endif

			/*
			 * Call truncate(2) to test different test conditions.
			 * verify that it fails with return code -1 and sets
			 * appropriate errno.
			 */
			TEST(truncate(file_name, TRUNC_LEN));

			/* check return code of truncate(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS, "truncate() fails, %s, "
						 "errno=%d", test_desc,
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "truncate() fails, %s, "
						 "errno=%d, expected errno:%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "truncate() returned %ld, "
					 "expected -1, errno:%d",
					 TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */
		Tst_count++;	/* incr TEST_LOOP counter */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and write some data
 *  into it, close it.
 *  Call individual test specific setup functions.
 */
void setup()
{
	int fd, i, ind;		/* file handler for testfile */
	int c, c_total = 0;	/* no. of bytes written to file */
	char tst_buff[BUF_SIZE];	/* buffer to hold data */

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* Creat a testfile and open it for reading/writing */
	if ((fd = open(TEST_FILE1, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TEST_FILE1, FILE_MODE, errno, strerror(errno));
	 /*NOTREACHED*/}

	/* Write to the file 1k data from the buffer */
	while (c_total < FILE_SIZE) {
		if ((c = write(fd, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK, cleanup,
				 "write(2) on %s Failed, errno=%d : %s",
				 TEST_FILE1, errno, strerror(errno));
		 /*NOTREACHED*/} else {
			c_total += c;
		}
	}

	/* Close the testfile after writing data into it */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s",
			 TEST_FILE1, errno, strerror(errno));
	 /*NOTREACHED*/}
#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	Test_cases[3].pathname = bad_addr;
#endif

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}				/* End setup() */

/*
 * int
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *              Hence, this function just returns 0.
 *  This function simply returns 0.
 */
int no_setup()
{
	return 0;
}

/*
 * int
 * setup1() - setup function for a test condition for which truncate(2)
 *            returns -1 and sets errno to EACCES.
 *  Modify the mode permissions on testfile such that process will not
 *  have write permissions on it.
 *
 *  The function returns 0.
 */
int setup1()
{
	/* Change mode permissions on test file */
	if (chmod(TEST_FILE1, NEW_MODE) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", TEST_FILE1);
	 /*NOTREACHED*/}

	return 0;
}				/* End setup() */

/*
 * int
 * setup2() - setup function for a test condition for which truncate(2)
 *           returns -1 and sets errno to ENOTDIR.
 *
 *  Create a test file under temporary directory so that test tries to
 *  truncate testfile under "t_file" which happens to be another regular file.
 *  The function returns 0.
 */
int setup2()
{
	int fildes;

	/* creat/open a test file under temporary directory */
	if ((fildes = open("t_file", O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(2) on t_file failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}
	/* Close the file created above */
	if (close(fildes) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(t_file) Failed, errno=%d : %s",
			 errno, strerror(errno));
	 /*NOTREACHED*/}
	return 0;
}

/*
 * int
 * longpath_setup() - setup to create a node with a name length exceeding
 *                    the MAX. length of PATH_MAX.
 *   This function retruns 0.
 */
int longpath_setup()
{
	int ind;		/* counter variable */

	for (ind = 0; ind <= (PATH_MAX + 1); ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;
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

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
