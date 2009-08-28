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
 * Test Name: fstat02
 *
 * Test Description:
 *  Verify that, fstat(2) succeeds to get the status of a file and fills
 *  the stat structure elements though file pointed to by file descriptor
 *  not opened for reading.
 *
 * Expected Result:
 *  fstat() should return value 0 on success and the stat structure elements
 *  should be filled with specified 'file' information.
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
 *  fstat02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define FILE_MODE	0644
#define TESTFILE	"testfile"
#define FILE_SIZE       1024
#define BUF_SIZE	256
#define MASK		0777

char *TCID = "fstat02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
uid_t User_id;			/* user id/group id of test process */
gid_t Group_id;
int fildes;			/* File descriptor of testfile */

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Setup function for the test */
void cleanup();			/* Cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat structure buffer */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

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

		/*
		 * Call fstat(2) to get the status of
		 * specified 'file' pointed to by 'fd'
		 * into stat structure.
		 */
		TEST(fstat(fildes, &stat_buf));

		/* check return code of fstat(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "fstat(%s) failed", TESTFILE);
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Verify the data returned by fstat(2)
			 * aganist the expected data.
			 */
			if ((stat_buf.st_uid != User_id) ||
			    (stat_buf.st_gid != Group_id) ||
			    (stat_buf.st_size != FILE_SIZE) ||
			    ((stat_buf.st_mode & MASK) != FILE_MODE)) {
				tst_resm(TFAIL, "Functionality of fstat(2) on "
					 "'%s' Failed", TESTFILE);
			} else {
				tst_resm(TPASS, "Functionality of fstat(2) on "
					 "'%s' Succcessful", TESTFILE);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * void
 * setup() -  Performs setup function for the test.
 *  Creat a temporary directory and chdir to it.
 *  Creat a temporary file and write some known data into it.
 *  Get the effective uid/gid of test process.
 */
void setup()
{
	int i;
	char tst_buff[BUF_SIZE];
	int wbytes;
	int write_len = 0;

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

	fildes = open(TESTFILE, O_WRONLY | O_CREAT, FILE_MODE);
	if (fildes == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TESTFILE, FILE_MODE);

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* Write to the file 1k data from the buffer */
	while (write_len < FILE_SIZE) {
		if ((wbytes = write(fildes, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK|TERRNO, cleanup,
				 "write(%s) failed", TESTFILE);
		} else {
			write_len += wbytes;
		}
	}

	/* Get the uid/gid of the process */
	User_id = getuid();
	Group_id = getgid();

}				/* End setup() */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the test file and remove the test file and temporary directory.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the test file */
	if (close(fildes) == -1)
		tst_brkm(TFAIL|TERRNO, NULL, "close(%s) failed", TESTFILE);

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
