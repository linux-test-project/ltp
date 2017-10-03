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
 * Test Name: pread01
 *
 * Test Description:
 *  Verify the functionality of pread() by writing known data using pwrite()
 *  to the file at various specified offsets and later read from the file from
 *  various specified offsets, comparing the data read aganist the data
 *  written.
 *
 * Expected Result:
 *  pread() should succeed to read the expected no. of bytes of data and
 *  the data read should match aganist the data written to the file.
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
 *      Issue a FAIL message.
 *   Otherwise,
 *      Verify the Functionality of system call
 *      if successful,
 *          Issue Functionality-Pass message.
 *      Otherwise,
 *          Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  pread01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 */

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "test.h"
#include "safe_macros.h"

#define TEMPFILE	"pread_file"
#define K1              1024
#define K2              (K1 * 2)
#define K3              (K1 * 3)
#define K4              (K1 * 4)
#define NBUFS           4

char *TCID = "pread01";
int TST_TOTAL = 1;

int fildes;			/* file descriptor for tempfile */
char *write_buf[NBUFS];		/* buffer to hold data to be written */
char *read_buf[NBUFS];		/* buffer to hold data read from file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void l_seek(int, off_t, int, off_t);	/* function to call lseek() */
void init_buffers();		/* function to initialize/allocate buffers */
void compare_bufers();		/* function to compare o/p of pread/pwrite */

int main(int ac, char **av)
{
	int lc;
	int nread;		/* no. of bytes read by pread() */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * Call pread() of K1 data (should be 2's) at offset K2.
		 */
		nread = pread(fildes, read_buf[2], K1, K2);

		/* Check for the return value of pread() */
		if (nread != K1) {
			tst_brkm(TFAIL, cleanup, "pread() at off. K2 failed: "
				 "nread=%d, error:%d", nread, errno);
		}

		/*
		 * We should still be at offset K4,
		 * which we were at the end of block 0.
		 */
		l_seek(fildes, 0, SEEK_CUR, K4);

		/* Now lseek() to offset 0. */
		l_seek(fildes, 0, SEEK_SET, 0);

		/* pread() K1 of data (should be 3's) at offset K3. */
		nread = pread(fildes, read_buf[3], K1, K3);
		if (nread != K1) {
			tst_brkm(TFAIL, cleanup, "pread() at off. K3 failed: "
				 "nread=%d, error:%d", nread, errno);
		}

		/* We should still be at offset 0. */
		l_seek(fildes, 0, SEEK_CUR, 0);

		/*
		 * Do a normal read() of K1 data (should be 0's)
		 * which should take place at offset 0 and move the
		 * file pointer to an offset of K1.
		 */
		if ((nread = read(fildes, read_buf[0], K1)) != K1) {
			tst_brkm(TFAIL, cleanup, "read() at off. 0 failed: "
				 "nread=%d, errno=%d", nread, errno);
		}

		/* We should now be at an offset of K1. */
		l_seek(fildes, 0, SEEK_CUR, K1);

		/* pread() of K1 data (should be 1's) at offset K1. */
		nread = pread(fildes, read_buf[1], K1, K1);
		if (nread != K1) {
			tst_brkm(TFAIL, cleanup, "pread() at off. K1 failed: "
				 "nread=%d, error:%d", nread, errno);
		}

		/* We should still be at offset K1. */
		l_seek(fildes, 0, SEEK_CUR, K1);

		/*
		 * Compare the read buffer data read
		 * with the data written to write buffer
		 * in the setup.
		 */
		compare_bufers();

		/* reset our location to offset K4 in case we are looping */
		l_seek(fildes, K4, SEEK_SET, K4);
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Initialize/allocate read/write buffers.
 *  Create a temporary directory and a file under it and
 *  write know data at different offset positions.
 */
void setup(void)
{
	int nwrite = 0;		/* no. of bytes written by pwrite() */

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate/Initialize the read/write buffer with know data */
	init_buffers();

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* pwrite() K1 of data (0's) at offset 0 of temporary file */
	if ((nwrite = pwrite(fildes, write_buf[0], K1, 0)) != K1) {
		tst_brkm(TBROK, cleanup, "pwrite() failed to write on %s, "
			 "errno=%d : %s", TEMPFILE, errno, strerror(errno));
	}

	/* We should still be at offset 0. */
	l_seek(fildes, 0, SEEK_CUR, 0);

	/* Now, lseek() to a non K boundary, just to be different. */
	l_seek(fildes, K1 / 2, SEEK_SET, K1 / 2);

	/* Again, pwrite() K1 of data (2's) at offset K2 of temporary file */
	if ((nwrite = pwrite(fildes, write_buf[2], K1, K2)) != K1) {
		tst_brkm(TBROK, cleanup, "pwrite() failed to write at %d off. "
			 "on %s, errno=%d : %s", K2, TEMPFILE, errno,
			 strerror(errno));
	}

	/* We should still be at our non K boundary. */
	l_seek(fildes, 0, SEEK_CUR, K1 / 2);

	/* lseek() to an offset of K3. */
	l_seek(fildes, K3, SEEK_SET, K3);

	/*
	 * Using write(), write of K1 of data (3's) which should take
	 * place at an offset of K3, moving the file pointer to K4.
	 */
	if ((nwrite = write(fildes, write_buf[3], K1)) != K1) {
		tst_brkm(TBROK, cleanup, "write() failed: nwrite=%d, errno=%d "
			 ": %s", nwrite, errno, strerror(errno));
	}

	/* We should be at offset K4. */
	l_seek(fildes, 0, SEEK_CUR, K4);

	/* Again, pwrite() K1 of data (1's) at offset K1. */
	if ((nwrite = pwrite(fildes, write_buf[1], K1, K1)) != K1) {
		tst_brkm(TBROK, cleanup, "pwrite() failed to write at %d off. "
			 "on %s, errno=%d : %s", K1, TEMPFILE, errno,
			 strerror(errno));
	}
}

/*
 * init_buffers - allocates both write_buf and read_buf arrays.
 *
 *  Allocate the read and write buffers.
 *  Fill the write buffer with the following data like,
 *    write_buf[0] has 0's, write_buf[1] has 1's, write_buf[2] has 2's
 *    write_buf[3] has 3's.
 */
void init_buffers(void)
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize read/write buffer */
	for (count = 0; count < NBUFS; count++) {
		write_buf[count] = malloc(K1);
		read_buf[count] = malloc(K1);

		if ((write_buf[count] == NULL) || (read_buf[count] == NULL)) {
			tst_brkm(TBROK, NULL,
				 "malloc() failed on read/write buffers");
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
		tst_resm(TWARN, "return = %" PRId64 ", expected %" PRId64,
			 (int64_t) offloc, (int64_t) checkoff);
		tst_brkm(TBROK | TERRNO, cleanup, "lseek() on %s failed",
			 TEMPFILE);
	}
}

/*
 * compare_bufers() - Compare the contents of read buffer aganist the
 *                    write buffer contents.
 *
 *  The contents of the index of each buffer should be as follows:
 *  [0] has 0's, [1] has 1's, [2] has 2's, and [3] has 3's.
 *
 *  This function does memcmp of read/write buffer and display message
 *  about the functionality of pread().
 */
void compare_bufers(void)
{
	int count;		/* index for the loop */
	int err_flg = 0;	/* flag to indicate error */

	for (count = 0; count < NBUFS; count++) {
		if (memcmp(write_buf[count], read_buf[count], K1) != 0) {
			tst_resm(TFAIL, "read/write buffer data mismatch");
			err_flg++;
		}
	}

	/* If no erros, Test successful */
	if (!err_flg) {
		tst_resm(TPASS, "Functionality of pread() is correct");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *             Deallocate the memory allocated to read/write buffers.
 *             Close the temporary file.
 *             Remove the temporary directory created.
 */
void cleanup(void)
{
	int count;

	/* Free the memory allocated for the read/write buffer */
	for (count = 0; count < NBUFS; count++) {
		free(write_buf[count]);
		free(read_buf[count]);
	}

	/* Close the temporary file */
	SAFE_CLOSE(NULL, fildes);

	tst_rmdir();

}
