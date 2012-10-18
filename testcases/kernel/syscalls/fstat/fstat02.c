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
uid_t user_id;			/* user id/group id of test process */
gid_t group_id;
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

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(fstat(fildes, &stat_buf));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "fstat(%s) failed", TESTFILE);
			continue;
		}
		if (STD_FUNCTIONAL_TEST) {
			if (stat_buf.st_uid != user_id ||
			    stat_buf.st_gid != group_id ||
			    stat_buf.st_size != FILE_SIZE ||
			    (stat_buf.st_mode & MASK) != FILE_MODE) {
				tst_resm(TFAIL,
				    "functionality of fstat incorrect");
			} else
				tst_resm(TPASS,
				    "functionality of fstat correct");
		} else
			tst_resm(TPASS, "call succeeded");
	}

	cleanup();

	tst_exit();
}

void setup()
{
	char tst_buff[BUF_SIZE];
	int wbytes;
	int write_len = 0;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "setuid failed");

	TEST_PAUSE;

	tst_tmpdir();

	fildes = open(TESTFILE, O_WRONLY|O_CREAT, FILE_MODE);
	if (fildes == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'a', BUF_SIZE-1);

	/* Write to the file 1k data from the buffer */
	while (write_len < FILE_SIZE) {
		if ((wbytes = write(fildes, tst_buff, sizeof(tst_buff))) <= 0)
			tst_brkm(TBROK|TERRNO, cleanup, "write failed");
		else
			write_len += wbytes;
	}

	user_id = getuid();
	group_id = getgid();

}

void cleanup()
{
	TEST_CLEANUP;

	if (close(fildes) == -1)
		tst_resm(TWARN|TERRNO, "close failed");

	tst_rmdir();

}
