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
 * Test Name: msync01
 *
 * Test Description:
 *  Verify that, msync() succeeds, when the region to synchronize, is part
 *  of, or all of a mapped region.
 *
 * Expected Result:
 *  msync() should succeed with a return value of 0, and succesfully
 *  synchronize the memory region.  Data read from mapped region should be
 *  the same as the initialized data.
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
 *  msync01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"msync_file"
#define BUF_SIZE	256

char *TCID = "msync01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */
char write_buf[BUF_SIZE];	/* buffer to hold data to be written */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char read_buf[BUF_SIZE];	/* buffer to hold data read from file */
	int nread = 0, count, err_flg = 0;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		setup();

		TEST(msync(addr, page_sz, MS_ASYNC));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TERRNO, "msync failed");
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			if (lseek(fildes, (off_t) 100, SEEK_SET) != 100)
				tst_brkm(TBROK|TERRNO, cleanup, "lseek failed");

			/*
			 * Seeking to specified offset. successful.
			 * Now, read the data (256 bytes) and compare
			 * them with the expected.
			 */
			nread = read(fildes, read_buf, sizeof(read_buf));
			if (nread != BUF_SIZE)
				tst_brkm(TBROK, cleanup, "read failed");
			else {
				/*
				 * Check whether read data (from mapped
				 * file) contains the expected data
				 * which was initialised in the setup.
				 */
				for (count = 0; count < nread; count++)
					if (read_buf[count] != 1)
						err_flg++;
			}

			if (err_flg != 0)
				tst_resm(TFAIL,
					 "data read from file doesn't match");
			else
				tst_resm(TPASS,
					 "Functionality of msync() successful");
		} else
			tst_resm(TPASS, "call succeeded");

		cleanup();

	}
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 * Get system page size,
 * Creat a temporary directory and a file under it used for mapping.
 * Write 1 page size char data into file.
 * Initialize paged region (256 bytes) from the specified offset pos.
 */
void setup()
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if ((page_sz = getpagesize()) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "getpagesize failed");

	tst_tmpdir();

	if ((fildes = open(TEMPFILE, O_RDWR|O_CREAT, 0666)) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	while (c_total < page_sz) {
		nwrite = write(fildes, write_buf, sizeof(write_buf));
		if (nwrite <= 0)
			tst_brkm(TBROK|TERRNO, cleanup, "write failed");
		else
			c_total += nwrite;
	}

	/*
	 * Call mmap to map virtual memory (mul. of page size bytes) from the
	 * beginning of temporary file (offset is 0) into memory.
	 */
	addr = mmap(0, page_sz, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,
		    fildes, 0);

	/* Check for the return value of mmap() */
	if (addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");

	/* Set 256 bytes, at 100 byte offset in the mapped region */
	memset(addr + 100, 1, 256);
}

void cleanup()
{
	TEST_CLEANUP;

	if (munmap(addr, page_sz) == -1)
		tst_resm(TBROK|TERRNO, "munmap failed");

	if (close(fildes) == -1)
		tst_resm(TWARN|TERRNO, "close failed");

	tst_rmdir();
}