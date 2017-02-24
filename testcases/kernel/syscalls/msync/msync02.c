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
 * Test Name: msync02
 *
 * Test Description:
 *  Verify that msync() succeeds when the region to synchronize is mapped
 *  shared and the flags argument is MS_INVALIDATE.
 *
 * Expected Result:
 *  msync() should succeed with a return value of 0, and successfully
 *  synchronize the memory region.
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
 *  msync02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define TEMPFILE	"msync_file"
#define BUF_SIZE	256

char *TCID = "msync02";
int TST_TOTAL = 1;

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */
char write_buf[10] = "Testing";	/* buffer to hold data to be written */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	TEST(msync(addr, page_sz, MS_INVALIDATE));

	if (TEST_RETURN == -1)
		tst_resm(TFAIL | TTERRNO, "msync failed");
	else if (memcmp(addr + 100, write_buf, strlen(write_buf)) != 0)
		tst_resm(TFAIL, "memory region contains invalid data");
	else
		tst_resm(TPASS, "Functionality of msync successful");

	cleanup();
	tst_exit();
}

void setup(void)
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */
	char tst_buf[BUF_SIZE];

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if ((page_sz = getpagesize()) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "getpagesize failed");

	tst_tmpdir();

	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "open failed");

	/* Write one page size of char data into temporary file */
	while (c_total < page_sz) {
		if ((nwrite = write(fildes, tst_buf, sizeof(tst_buf))) <= 0)
			tst_brkm(TBROK | TERRNO, cleanup, "write failed");
		else
			c_total += nwrite;
	}

	addr = mmap(0, page_sz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
		    fildes, 0);

	if (addr == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");

	/* Again, Seek to the specified byte offset (100) position */
	if (lseek(fildes, 100, SEEK_SET) != 100)
		tst_brkm(TBROK | TERRNO, cleanup, "lseek failed");

	/* Write the string in write_buf at the 100 byte offset */
	if (write(fildes, write_buf, strlen(write_buf)) != strlen(write_buf))
		tst_brkm(TBROK | TERRNO, cleanup, "write failed");
}

void cleanup(void)
{
	if (munmap(addr, page_sz) == -1)
		tst_resm(TBROK | TERRNO, "munmap failed");

	/* Close the temporary file */
	if (close(fildes) == -1)
		tst_resm(TWARN | TERRNO, "close failed");

	tst_rmdir();
}
