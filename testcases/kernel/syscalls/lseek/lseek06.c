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
 * Test Name: lseek06
 *
 * Test Description:
 *  Verify that, lseek() call succeeds to set the file pointer position
 *  to less  than  or equal to the file size, when a file is opened for
 *  read or write.
 *
 * Expected Result:
 *  lseek() should return the offset from the beginning of the file measured
 *  in bytes. Also check if able to read valid data from this location.
 * $
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
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
 *  lseek06 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define OFFSET		4
#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "lseek06";
int TST_TOTAL = 1;
int fildes;			/* file handle for temp file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	char read_buf[1];	/* data read from temp. file */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Invoke lseek(2) to move the read/write file
		 * pointer/handle by OFFSET bytes.
		 */
		TEST(lseek(fildes, OFFSET, SEEK_SET));

		if (TEST_RETURN == (off_t) - 1) {
			tst_resm(TFAIL, "lseek on (%s) Failed, errno=%d : %s",
				 TEMP_FILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Check if the return value from lseek(2)
		 * is equal to the specified OFFSET value.
		 */
		if (TEST_RETURN != OFFSET) {
			tst_resm(TFAIL, "lseek() returned incorrect "
				 "value %ld, expected %d",
				 TEST_RETURN, OFFSET);
			continue;
		}
		/*
		 * The return value is good, now check data.
		 * Read the data byte from this location.
		 */
		if (read(fildes, &read_buf, sizeof(read_buf)) < 0) {
			tst_brkm(TFAIL, cleanup, "read() failed "
				 "on %s, error=%d", TEMP_FILE, errno);
		} else {
			/*
			 * Check if read byte is the expected character.
			 * For pos 4 ---> 'e'
			 */
			if (read_buf[0] != 'e') {
				tst_resm(TFAIL, "Incorrect data read "
					 "from file %s", TEMP_FILE);
			} else {
				tst_resm(TPASS, "Functionality "
					 "of lseek() on %s "
					 "successful", TEMP_FILE);
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Create a temporary directory and change directory to it.
 *	     Create a test file under temporary directory and write some
 *	     data into it.
 */
void setup(void)
{
	char write_buf[BUFSIZ];	/* buffer to hold data */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

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
	if (write(fildes, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TBROK, cleanup, "write(2) on %s Failed, errno=%d : %s",
			 TEMP_FILE, errno, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	/* Close the temporary file created */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, NULL, "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	}

	tst_rmdir();

}
