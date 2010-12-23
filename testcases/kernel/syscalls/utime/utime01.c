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
 * Test Name: utime01
 *
 * Test Description:
 *  Verify that the system call utime() successfully sets the modification
 *  and access times of a file to the current time, if the times argument
 *  is null, and the user ID of the process is "root".
 *
 * Expected Result:
 *  utime succeeds returning zero and sets the access and modification
 *  times of the file to the current time.
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
 *  utime01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-f   : Turn off functionality Testing.
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IRGRP | S_IROTH

char *TCID = "utime01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
time_t curr_time;		/* current time in seconds */
time_t tloc;			/* argument var. for time() */
int exp_enos[] = { 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer to hold file info. */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	time_t modf_time, access_time;
	time_t pres_time;	/* file modification/access/present time */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	/*
	 * check if the current filesystem is nfs
	 */
	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do utime on a file located on an NFS filesystem");
	}

        if (tst_is_cwd_v9fs()) {
                tst_brkm(TCONF, cleanup,
                         "Cannot do utime on a file located on an 9P filesystem");
        }

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Invoke utime(2) to set TEMP_FILE access and
		 * modification times to the current time.
		 */
		TEST(utime(TEMP_FILE, NULL));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "utime(%s) Failed, errno=%d : %s",
				 TEMP_FILE, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Sleep for a second so that mod time and
				 * access times will be different from the
				 * current time
				 */
				sleep(2);

				/*
				 * Get the current time now, after calling
				 * utime(2)
				 */
				if ((pres_time = time(&tloc)) < 0) {
					tst_brkm(TFAIL, cleanup, "time() "
						 "failed to get present time "
						 "after utime, error=%d",
						 errno);
				 }

				/*
				 * Get the modification and access times of
				 * temporary file using stat(2).
				 */
				if (stat(TEMP_FILE, &stat_buf) < 0) {
					tst_brkm(TFAIL, cleanup, "stat(2) of "
						 "%s failed, error:%d",
						 TEMP_FILE, TEST_ERRNO);
				 }
				modf_time = stat_buf.st_mtime;
				access_time = stat_buf.st_atime;

				/* Now do the actual verification */
				if (modf_time <= curr_time ||
				    modf_time >= pres_time ||
				    access_time <= curr_time ||
				    access_time >= pres_time) {
					tst_resm(TFAIL, "%s access and "
						 "modification times not set",
						 TEMP_FILE);
				} else {
					tst_resm(TPASS, "Functionality of "
						 "utime(%s, NULL) successful",
						 TEMP_FILE);
				}
			} else {
				tst_resm(TPASS, "%s call succeeded", TCID);
			}
		}
		Tst_count++;
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 */
void setup()
{
	int fildes;		/* file handle for temp file */

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	TEST_PAUSE;

	tst_tmpdir();

	/* Creat a temporary file under above directory */
	if ((fildes = creat(TEMP_FILE, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(%s, %#o) Failed, errno=%d :%s",
			 TEMP_FILE, FILE_MODE, errno, strerror(errno));
	 }

	/* Close the temporary file created */
	if (close(fildes) < 0) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	 }

	/* Get the current time */
	if ((curr_time = time(&tloc)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "time() failed to get current time, errno=%d", errno);
	 }

	/*
	 * Sleep for a second so that mod time and access times will be
	 * different from the current time
	 */
	sleep(2);		/* sleep(1) on IA64 sometimes sleeps < 1 sec!! */

}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
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