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
 * Test Name: utime05
 *
 * Test Description:
 *  Verify that the system call utime() successfully sets the modification
 *  and access times of a file to the value specified by the times argument
 *  under the following constraints,
 *		- The times argument is not null,
 *		- The user ID of the process is not "root".
 *		- The file is owned by the user ID of the process.
 *
 * Expected Result:
 *  utime succeeds returning zero and sets the access and modification
 *  times of the file to that specified by the times argument.
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
 *  utime05 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *		where,	-c n : Run n copies concurrently.
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
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IRGRP | S_IROTH
#define NEW_TIME	10000

char *TCID = "utime05";
int TST_TOTAL = 1;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct utimbuf times;		/* struct. buffer for utime() */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer to hold file info. */
	int lc;
	time_t modf_time, access_time;
	/* file modification/access time */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Invoke utime(2) to set TEMP_FILE access and
		 * modification times to that specified by
		 * times argument.
		 */
		TEST(utime(TEMP_FILE, &times));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "utime(%s) failed", TEMP_FILE);
		} else {
			/*
			 * Get the modification and access times of
			 * temporary file using stat(2).
			 */
			SAFE_STAT(cleanup, TEMP_FILE, &stat_buf);
			modf_time = stat_buf.st_mtime;
			access_time = stat_buf.st_atime;

			/* Now do the actual verification */
			if ((modf_time != NEW_TIME) ||
			    (access_time != NEW_TIME)) {
				tst_resm(TFAIL, "%s access and "
					 "modification times not set",
					 TEMP_FILE);
			} else {
				tst_resm(TPASS, "Functionality of "
					 "utime(%s, &times) successful",
					 TEMP_FILE);
			}
		}
		tst_count++;	/* incr TEST_LOOP counter */
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
void setup(void)
{
	int fildes;		/* file handle for temp file */

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	ltpuser = SAFE_GETPWNAM(NULL, nobody_uid);
	SAFE_SETUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	/* Creat a temporary file under above directory */
	fildes = SAFE_CREAT(cleanup, TEMP_FILE, FILE_MODE);

	/* Close the temporary file created */
	SAFE_CLOSE(cleanup, fildes);

	/* Initialize the modification and access time in the times arg */
	times.actime = NEW_TIME;
	times.modtime = NEW_TIME;

}

/*
 * void
 * cleanup() -	performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *		Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	tst_rmdir();

}
