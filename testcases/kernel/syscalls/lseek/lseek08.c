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
 * Test Name: lseek08
 *
 * Test Description:
 *  Verify that, lseek() call succeeds to set the file pointer position
 *  to the end of the file when 'whence' value set to SEEK_END and any
 *  attempts to read from that position should fail.
 *
 * Expected Result:
 *  lseek() should return the offset which is set to the file size measured
 *  in bytes. read() attempt should fail with -1 return value.
 * $
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
 *  lseek08 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "lseek08";
int TST_TOTAL = 1;
int fildes;			/* file handle for temp file */
size_t file_size;		/* size of the temporary file */

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
		 * pointer/handle to the END of the file.
		 */
		TEST(lseek(fildes, 0, SEEK_END));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				 "lseek of %s failed", TEMP_FILE);
			continue;
		}
		/*
		 * Check if the return value from lseek(2)
		 * is equal to the file_size.
		 */
		if (TEST_RETURN != file_size) {
			tst_resm(TFAIL, "lseek() returned incorrect "
				 "value %ld, expected %zu",
				 TEST_RETURN, file_size);
			continue;
		}
		/*
		 * The return value is okay, now attempt to read data
		 * from the file.  This should fail as the file pointer
		 * should be pointing to END OF FILE.
		 */
		read_buf[0] = '\0';
		if (read(fildes, &read_buf, sizeof(read_buf)) > 0) {
			tst_resm(TFAIL, "read() successful on %s",
				 TEMP_FILE);
		} else {
			tst_resm(TPASS, "Functionality of lseek() on "
				 "%s successful", TEMP_FILE);
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
	struct stat stat_buf;	/* struct. buffer for stat(2) */
	char write_buf[BUFSIZ];	/* buffer to hold data */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get the data to be written to temporary file */
	strcpy(write_buf, "abcdefg\n");

	/* Creat/open a temporary file under above directory */
	if ((fildes = open(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEMP_FILE, FILE_MODE);
	}

	/* Write data into temporary file */
	if (write(fildes, write_buf, strlen(write_buf)) <= 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "writing to %s failed", TEMP_FILE);
	}

	/* Get the size of the file using fstat */
	if (fstat(fildes, &stat_buf) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "fstat of %s failed", TEMP_FILE);
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
		tst_brkm(TFAIL, NULL,
			 "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE, errno, strerror(errno));
	}

	tst_rmdir();

}
