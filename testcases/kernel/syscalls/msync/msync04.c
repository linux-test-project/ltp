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
 * Test Name: msync04
 *
 * Test Description:
 *  Verify that, msync() fails, when the region to synchronize, is mapped
 *  but the flags argument is invalid.
 *
 * Expected Result:
 *  msync() should fail with a return value of -1, and set errno EINVAL.
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
 *      if errno set == expected errno
 *              Issue sys call fails with expected return value and errno.
 *      Otherwise,
 *              Issue sys call fails with unexpected errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  msync04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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
#define INV_SYNC	-1

char *TCID = "msync04";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */

int exp_enos[] = { EINVAL, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	TEST_EXP_ENOS(exp_enos);

	Tst_count = 0;

	setup();

	TEST(msync(addr, page_sz, INV_SYNC));

	if (TEST_RETURN != -1)
		tst_resm(TFAIL, "msync succeeded unexpectedly");
	else if (TEST_ERRNO == EINVAL)
		tst_resm(TPASS, "msync failed with EINVAL as expected");
	else
		tst_resm(TFAIL|TTERRNO, "msync failed unexpectedly");

	cleanup();

	tst_exit();
}

void setup()
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */
	char tst_buf[BUF_SIZE];	/* buffer to hold data to be written */

	TEST_PAUSE;

	if ((page_sz = getpagesize()) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "getpagesize failed");

	tst_tmpdir();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	/* Write one page size of char data into temporary file */
	while (c_total < page_sz) {
		if ((nwrite = write(fildes, tst_buf, sizeof(tst_buf))) <= 0)
			tst_brkm(TBROK, cleanup, "write failed");
		else
			c_total += nwrite;
	}

	addr = mmap(0, page_sz, PROT_READ|PROT_WRITE, MAP_FILE|MAP_PRIVATE,
		    fildes, 0);

	/* Check for the return value of mmap() */
	if (addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");
}

void cleanup()
{
	TEST_CLEANUP;

	if (munmap(addr, page_sz) == -1)
		tst_brkm(TBROK, NULL, "munmap failed");

	/* Close the temporary file */
	if (close(fildes) == -1)
		tst_brkm(TBROK, NULL, "close failed");

	tst_rmdir();
}