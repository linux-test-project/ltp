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
 * Test Name: lseek07
 *
 * Test Description:
 *  Verify that, lseek() call succeeds to set the file pointer position
 *  to more than the file size, when a file is opened for reading/writing.
 *
 * Expected Result:
 *  lseek() should return n+1, where n is the size of the file.
 *  Also when some data is written into this file it should start
 *  from that offset.
 *
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
 *  lseek07 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <inttypes.h>

#include "test.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "lseek07";
int TST_TOTAL = 1;
int fildes;			/* file handle for temp file */
size_t file_size;		/* size of temporary file */
char write_buf1[BUFSIZ];	/* buffer to hold data */
char write_buf2[BUFSIZ];	/* buffer to hold data */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	char read_buf[BUFSIZ];	/* data read from temp. file */
	off_t offset;		/* byte position in temporary file */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Set the offset position */
		offset = file_size + (lc * strlen(write_buf2));

		/*
		 * Invoke lseek(2) to move the write file
		 * pointer/handle by the specified offset value.
		 */
		TEST(lseek(fildes, offset, SEEK_SET));

		if (TEST_RETURN == (off_t) - 1) {
			tst_resm(TFAIL | TTERRNO, "lseek on (%s) failed",
				 TEMP_FILE);
			continue;
		}
		/*
		 * Check if the return value from lseek(2)
		 * is equal to the specified offset value.
		 */
		if (TEST_RETURN != offset) {
			tst_resm(TFAIL, "lseek() returned "
				 "incorrect value %ld, expected "
				 "%" PRId64, TEST_RETURN,
				 (int64_t) offset);
			continue;
		}
		/*
		 * The return value is okay, now write some data at
		 * the current offset position.
		 */
		if (write(fildes, write_buf2, strlen(write_buf2)) !=
		    strlen(write_buf2)) {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "write() failed to write additional data");
		}

		/*
		 * Now close the file and open it again
		 * and read all of the data.
		 */
		if (close(fildes) < 0) {
			tst_brkm(TFAIL, cleanup, "close() on %s Failed,"
				 " errno = %d", TEMP_FILE, errno);
		}

		/* Open the file again in read/write mode */
		if ((fildes = open(TEMP_FILE, O_RDWR)) < 0) {
			tst_brkm(TFAIL, cleanup, "Could not open the "
				 "%s readonly, error = %d",
				 TEMP_FILE, errno);
		}

		/*
		 * Now read all of the data.  The size should be the
		 * offset + strlen(write_buf2).
		 */
		if (read(fildes, &read_buf, (offset +
					     strlen(write_buf2))) < 0) {
			tst_brkm(TFAIL, cleanup, "read() failed on %s, "
				 "error=%d", TEMP_FILE, errno);
		} else {
			/*
			 * Check data read is the complete data and not
			 * the only portion written.
			 */
			if ((strncmp(read_buf, write_buf1,
				     strlen(write_buf1))) != 0) {
				tst_brkm(TFAIL, cleanup,
					 "Incorrect data read #1 from "
					 "file %s", TEMP_FILE);
			}
			if ((strncmp(&read_buf[offset], write_buf2,
				     strlen(write_buf2))) != 0) {
				tst_brkm(TFAIL, cleanup,
					 "Incorrect data read #2 from "
					 "file %s", TEMP_FILE);
			}
			tst_resm(TPASS, "Functionality of "
				 "lseek() on %s successful", TEMP_FILE);
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
 *	     Get the size of the file using fstat().
 */
void setup(void)
{
	struct stat stat_buf;	/* struct buffer for stat(2) */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get the data to be written to temporary file */
	strcpy(write_buf1, "abcdefg");
	strcpy(write_buf2, "ijk");

	/* Creat/open a temporary file for writing under above directory */
	if ((fildes = open(TEMP_FILE, O_WRONLY | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_WRONLY|O_CREAT, %#o) Failed, errno=%d :%s",
			 TEMP_FILE, FILE_MODE, errno, strerror(errno));
	}

	/* Write data into temporary file */
	if (write(fildes, write_buf1, strlen(write_buf1)) != strlen(write_buf1)) {
		tst_brkm(TBROK, cleanup, "write(2) on %s Failed, errno=%d : %s",
			 TEMP_FILE, errno, strerror(errno));
	}

	/* Get the size of the temporary file after writing data */
	if (fstat(fildes, &stat_buf) < 0) {
		tst_brkm(TBROK, cleanup, "fstat() on %s Failed, errno=%d : %s",
			 TEMP_FILE, errno, strerror(errno));
	}

	file_size = stat_buf.st_size;
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
