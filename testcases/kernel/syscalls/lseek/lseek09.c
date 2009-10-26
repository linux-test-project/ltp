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
 * Test Name: lseek09
 *
 * Test Description:
 *  Verify that, lseek() call succeeds to set the file pointer position
 *  to the current specified location, when 'whence' value is set to
 *  SEEK_CUR and the data read from the specified location should match
 *  the expected data.
 *
 * Expected Result:
 *  lseek() should return the specified offset from the beginning of the file
 *  measured in bytes. The data read from this location should match the
 *  expected data.
 *$
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
 *  lseek09 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "lseek09";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int fildes;			/* file handle for temp file */
size_t file_size;		/* total size of file after data write */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char read_buf[BUFSIZ];	/* data read from temp. file */

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
		 * Invoke lseek(2) to set the file
		 * pointer/handle from the current location
		 * of the file descriptor + specified offset.
		 */
		TEST(lseek(fildes, 1, SEEK_CUR));

		/* check return code of lseek(2) */
		if (TEST_RETURN == (off_t) - 1) {
			tst_resm(TFAIL, "lseek on (%s) Failed, errno=%d : %s",
				 TEMP_FILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Check if the return value from lseek(2) is equal
			 * to the 3 positions from the beginning of the file.
			 * ie, 2 positions from lseek() in the setup +
			 * 1 position from above above.
			 */
			if (TEST_RETURN != 3) {
				tst_resm(TFAIL, "lseek() returned incorrect "
					 "value %ld, expected 4", TEST_RETURN);
				continue;
			}
			/*
			 * Read the data byte from this location.
			 */
			memset(read_buf, 0, sizeof(read_buf));
			if (read(fildes, &read_buf, (file_size - 3)) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "read() failed on %s, error=%d",
					 TEMP_FILE, errno);
			} else {
				/*
				 * Check if read data contains
				 * expected characters
				 * From pos 4 ---> 'defg'.
				 */
				if (strcmp(read_buf, "defg")) {
					tst_resm(TFAIL, "Incorrect data read "
						 "from file %s", TEMP_FILE);
				} else {
					tst_resm(TPASS, "Functionality of "
						 "lseek() on %s successful",
						 TEMP_FILE);
				}
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* reset file pointer in case we are looping */
		if (lseek(fildes, 2, SEEK_SET) == -1) {
			tst_brkm(TBROK, cleanup, "lseek failed - could not "
				 "reset file pointer");
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Create a temporary directory and change directory to it.
 *	     Create a test file under temporary directory and write some
 *	     data into it.
 *	     Get the size of the file using fstat().
 */
void setup()
{
	struct stat stat_buf;	/* buffer to hold stat info. */
	char write_buf[BUFSIZ];	/* buffer to hold data */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Get the data to be written to temporary file */
	strcpy(write_buf, "abcdefg");

	/* Creat/open a temporary file under above directory */
	if ((fildes = open(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d :%s",
			 TEMP_FILE, FILE_MODE, errno, strerror(errno));
	}

	/* Write data into temporary file */
	if (write(fildes, write_buf, strlen(write_buf)) <= 0) {
		tst_brkm(TBROK, cleanup, "write(2) on %s Failed, errno=%d : %s",
			 TEMP_FILE, errno, strerror(errno));
	}

	/* Get the temporary file information */
	if (fstat(fildes, &stat_buf) < 0) {
		tst_brkm(TBROK, cleanup, "fstat(2) on %s Failed, errno=%d : %s",
			 TEMP_FILE, errno, strerror(errno));
	}

	file_size = stat_buf.st_size;

	/*
	 * Reset the file pointer position to the specified offset
	 * from the beginning of the file.
	 */
	if (lseek(fildes, 2, SEEK_SET) != 2) {
		tst_brkm(TBROK, cleanup,
			 "lseek() fails to set file ptr. to specified offset");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the temporary file created */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, NULL, "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
