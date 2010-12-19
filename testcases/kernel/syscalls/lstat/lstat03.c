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
 * Test Name: lstat03
 *
 * Test Description:
 *  Verify that, lstat(2) succeeds to get the status of a file pointed to by
 *  symlink and fills the stat structure elements.
 *
 * Expected Result:
 *  lstat() should return value 0 on success and the stat structure elements
 *  should be filled with the symlink file information.
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
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  lstat03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'non-super-user' only.
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

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"
#define SFILE		"sfile"
#define FILE_SIZE       1024
#define BUF_SIZE	256
#define PERMS		0644

char *TCID = "lstat03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
uid_t user_id;			/* user id/group id of test process */
gid_t group_id;

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
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call lstat(2) to get the status of
		 * symlink file into stat structure.
		 */
		TEST(lstat(SFILE, &stat_buf));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "lstat(%s, &stat_buf) Failed, errno=%d : %s",
				 SFILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Verify the data returned by lstat(2)
			 * aganist the expected data.
			 */
			if ((stat_buf.st_uid != user_id) ||
			    (stat_buf.st_gid != group_id) ||
			    (!(stat_buf.st_mode && S_IFLNK)) ||
			    (stat_buf.st_size != strlen(TESTFILE))) {
				tst_resm(TFAIL, "Functionality of lstat(2) on "
					 "'%s' Failed", SFILE);
			} else {
				tst_resm(TPASS, "Functionality of lstat(2) on "
					 "'%s' Succcessful", SFILE);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();
	tst_exit();
	tst_exit();

}

/*
 * setup() -  Performs setup function for the test.
 *	      Creat a temporary directory and chdir to it.
 *	      Creat a test file and write some known data into it.
 *	      Close the test file and creat a symlink of test file under
 *	      temporary directory.
 *	      Get uid/gid of test process.
 */
void setup()
{
	int i, fd;		/* file handle for test file */
	char tst_buff[BUF_SIZE];	/* data buffer */
	int wbytes;		/* no. of bytes to be written */
	int write_len = 0;	/* size of data */

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

	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* Write to the file 1k data from the buffer */
	while (write_len < FILE_SIZE) {
		if ((wbytes = write(fd, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK, cleanup,
				 "write(2) on %s Failed, errno=%d : %s",
				 TESTFILE, errno, strerror(errno));
		} else {
			write_len += wbytes;
		}
	}

	if (close(fd) == -1) {
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	}

	/* Create a symlink of testfile */
	if (symlink(TESTFILE, SFILE) < 0) {
		tst_brkm(TBROK, cleanup, "symlink() of %s Failed, errno=%d : "
			 "%s", TESTFILE, errno, strerror(errno));
	}

	/* Get the uid/gid of the process */
	user_id = getuid();
	group_id = getgid();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *	       Remove the symlink file, test file and temporary directory.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

}