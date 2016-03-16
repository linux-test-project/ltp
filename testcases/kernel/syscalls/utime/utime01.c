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
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IRGRP | S_IROTH

char *TCID = "utime01";
int TST_TOTAL = 1;
time_t curr_time;		/* current time in seconds */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer to hold file info. */
	int lc;
	long type;
	time_t modf_time, access_time;
	time_t pres_time;	/* file modification/access/present time */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	switch ((type = tst_fs_type(cleanup, "."))) {
	case TST_NFS_MAGIC:
		if (tst_kvercmp(2, 6, 18) < 0)
			tst_brkm(TCONF, cleanup, "Cannot do utime on a file"
				" on %s filesystem before 2.6.18",
				 tst_fs_type_name(type));
		break;
	case TST_V9FS_MAGIC:
		tst_brkm(TCONF, cleanup,
			 "Cannot do utime on a file on %s filesystem",
			 tst_fs_type_name(type));
		break;
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Invoke utime(2) to set TEMP_FILE access and
		 * modification times to the current time.
		 */
		TEST(utime(TEMP_FILE, NULL));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "utime(%s) failed", TEMP_FILE);
		} else {
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
			pres_time = time(NULL);

			/*
			 * Get the modification and access times of
			 * temporary file using stat(2).
			 */
			SAFE_STAT(cleanup, TEMP_FILE, &stat_buf);
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
		}
		tst_count++;
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

	TEST_PAUSE;

	tst_tmpdir();

	/* Creat a temporary file under above directory */
	fildes = SAFE_CREAT(cleanup, TEMP_FILE, FILE_MODE);

	/* Close the temporary file created */
	SAFE_CLOSE(cleanup, fildes);

	/* Get the current time */
	curr_time = time(NULL);

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
void cleanup(void)
{

	tst_rmdir();

}
