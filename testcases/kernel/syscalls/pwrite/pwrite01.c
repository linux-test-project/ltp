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
 * Test Name: pwrite01
 *
 * Test Description:
 *  Verify the functionality of pwrite() by writing known data using pwrite()
 *  to the file at various specified offsets and later read from the file from
 *  various specified offsets, comparing the data written aganist the data
 *  read using read().
 *
 * Expected Result:
 *  pwrite() should succeed to write the expected no. of bytes of data and
 *  the data written at specified offsets should match aganist the data read
 *  from that offset.
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
 *  pwrite01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "test.h"
#include "safe_macros.h"

#define _XOPEN_SOURCE 500
#define TEMPFILE	"pwrite_file"
#define K1              1024
#define K2              (K1 * 2)
#define K3              (K1 * 3)
#define K4              (K1 * 4)
#define NBUFS           4

char *TCID = "pwrite01";
int TST_TOTAL = 1;
int fildes;			/* file descriptor for tempfile */
char *write_buf[NBUFS];		/* buffer to hold data to be written */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void l_seek(int, off_t, int, off_t);	/* function to call lseek() */
void init_buffers();		/* function to initialize/allocate buffers */
void check_file_contents();	/* function to verify the contents of file */

int main(int ac, char **av)
{
	int lc;
	int nwrite;		/* no. of bytes written by pwrite() */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call pwrite() to write K1 bytes of data (0's) at offset 0
		 * of temporary file
		 */
		nwrite = pwrite(fildes, write_buf[0], K1, 0);

		/* Check for the return value of pwrite() */
		if (nwrite != K1) {
			tst_resm(TFAIL, "pwrite() at offset 0 failed, errno=%d "
				 ": %s", errno, strerror(errno));
			continue;
		}

		/* We should still be at offset 0. */
		l_seek(fildes, 0, SEEK_CUR, 0);

		/*
		 * Now, lseek() to a non K boundary,
		 * just to be different.
		 */
		l_seek(fildes, K1 / 2, SEEK_SET, K1 / 2);

		/*
		 * Again, pwrite() K1 of data (2's) at offset K2 of
		 * temporary file.
		 */
		nwrite = pwrite(fildes, write_buf[2], K1, K2);
		if (nwrite != K1) {
			tst_resm(TFAIL, "pwrite() failed to "
				 "write at %d off. on %s, errno=%d : %s",
				 K2, TEMPFILE, errno, strerror(errno));
			continue;
		}

		/* We should still be at our non K boundary. */
		l_seek(fildes, 0, SEEK_CUR, K1 / 2);

		/* lseek() to an offset of K3. */
		l_seek(fildes, K3, SEEK_SET, K3);

		/*
		 * Using write(), write of K1 of data (3's) which
		 * should take place at an offset of K3,
		 * moving the file pointer to K4.
		 */
		nwrite = write(fildes, write_buf[3], K1);
		if (nwrite != K1) {
			tst_resm(TFAIL, "write() failed: nwrite=%d, errno=%d : "
				 "%s", nwrite, errno, strerror(errno));
			continue;
		}

		/* We should be at offset K4. */
		l_seek(fildes, 0, SEEK_CUR, K4);

		/* Again, pwrite() K1 of data (1's) at offset K1. */
		nwrite = pwrite(fildes, write_buf[1], K1, K1);
		if (nwrite != K1) {
			tst_resm(TFAIL, "pwrite() failed to write at "
				 "%d off. on %s, errno=%d : %s",
				 K1, TEMPFILE, errno, strerror(errno));
			continue;
		}

		/*
		 * Check the contents of temporary file
		 * to which data written using pwrite().
		 * Compare the data read with the original
		 * write_buf[] contents.
		 */
		check_file_contents();

		/* reset to offset 0 in case we are looping */
		l_seek(fildes, 0, SEEK_SET, 0);

	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Initialize/allocate read/write buffers.
 *  Create a temporary directory and a file under it and
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate/Initialize the write buffer with known data */
	init_buffers();

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}
}

/*
 * init_buffers() - allocate/Initialize write_buf array.
 *
 *  Allocate write buffer.
 *  Fill the write buffer with the following data like,
 *    write_buf[0] has 0's, write_buf[1] has 1's, write_buf[2] has 2's
 *    write_buf[3] has 3's.
 */
void init_buffers(void)
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize write buffer with known data */
	for (count = 0; count < NBUFS; count++) {
		write_buf[count] = malloc(K1);

		if (write_buf[count] == NULL) {
			tst_brkm(TBROK, NULL, "malloc() failed ");
		}
		memset(write_buf[count], count, K1);
	}
}

/*
 * l_seek() - local front end to lseek().
 *
 *  "checkoff" is the offset at which we believe we should be at.
 *  Used to validate pread/pwrite don't move the offset.
 */
void l_seek(int fdesc, off_t offset, int whence, off_t checkoff)
{
	off_t offloc;		/* offset ret. from lseek() */

	if ((offloc = lseek(fdesc, offset, whence)) != checkoff) {
		tst_resm(TWARN, "lseek returned %" PRId64 ", expected %" PRId64,
			 (int64_t) offloc, (int64_t) checkoff);
		tst_brkm(TBROK | TERRNO, cleanup, "lseek() on %s Failed",
			 TEMPFILE);
	}
}

/*
 * check_file_contents() - Check the contents of the file we wrote with
 *			   pwrite()'s.
 *  The contents of the file are verified by using a plain read() and
 *  Compare the data read with the original write_buf[] contents.
 */
void check_file_contents(void)
{
	int count, err_flg = 0;	/* index variable and error flag */
	int nread;		/* return value of read() */
	off_t offloc;		/* offset. ret. by lseek() */
	char *read_buf;		/* buffer to hold read data */

	/* Allocate space for read buffer */
	read_buf = malloc(K1);
	if (read_buf == NULL) {
		tst_brkm(TBROK, cleanup, "malloc() failed on read buffer");
	}

	/* Seek to app. location of file and read the data, compare it */
	for (count = 0; count < NBUFS; count++) {
		/* Seek to specified offset position from beginning */
		offloc = lseek(fildes, count * K1, SEEK_SET);
		if (offloc != (count * K1)) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "lseek() failed: offloc=%" PRId64,
				 (int64_t) offloc);
		}

		/* Read the data from file into a buffer */
		nread = read(fildes, read_buf, K1);
		if (nread != K1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "read() failed: nread=%d", nread);
		}

		/* Compare the read data with the data written using pwrite */
		if (memcmp(write_buf[count], read_buf, K1) != 0) {
			tst_resm(TFAIL, "read/write buffer data mismatch");
			err_flg++;
		}
	}

	/* If no erros, Test successful */
	if (!err_flg) {
		tst_resm(TPASS, "Functionality of pwrite() successful");
	}

	/* Release the memory allocated before return */
	free(read_buf);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 * Deallocate the memory allocated to write buffer.
 * Close the temporary file.
 * Remove the temporary directory created.
 */
void cleanup(void)
{
	int count;

	/* Free the memory allocated for the write buffer */
	for (count = 0; count < NBUFS; count++) {
		free(write_buf[count]);
	}

	/* Close the temporary file */
	SAFE_CLOSE(NULL, fildes);

	tst_rmdir();

}
