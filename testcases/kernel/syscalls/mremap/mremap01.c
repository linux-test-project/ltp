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
 * Test Name: mremap01
 *
 * Test Description:
 *  Verify that, mremap() succeeds when used to expand the existing
 *  virtual memory mapped region to the requested size where the
 *  virtual memory area was previously mapped to a file using mmap().
 *
 * Expected Result:
 *  mremap() should succeed returning the address of new virtual memory area.
 *  The expanded mapped memory region should be accessible and able to
 *  synchronize with the file.
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
 *  mremap01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *      11/09/2001 Manoj Iyer (manjo@austin.ibm.com)
 *	Modified.
 *	- #include <linux/mman.h> should not be included as per man page for
 *	  mremap, #include <sys/mman.h> alone should do the job. But inorder
 *	  to include definition of MREMAP_MAYMOVE defined in bits/mman.h
 *	  (included by sys/mman.h) __USE_GNU needs to be defined.
 *	  There may be a more elegant way of doing this...
 *
 * RESTRICTIONS:
 *  None.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "test.h"
#include "tso_safe_macros.h"

#define TEMPFILE	"mremapfile"

char *TCID = "mremap01";
int TST_TOTAL = 1;
char *addr;			/* addr of memory mapped region */
int memsize;			/* memory mapped size */
int newsize;			/* new size of virtual memory block */
int fildes;			/* file descriptor for tempfile */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int ind;		/* counter variable */

	tst_parse_opts(ac, av, NULL, NULL);

	tst_count = 0;

	setup();

	/*
	 * Call mremap to expand the existing mapped
	 * memory region (memsize) by newsize limits.
	 */
	addr = mremap(addr, memsize, newsize, MREMAP_MAYMOVE);

	/* Check for the return value of mremap() */
	if (addr == MAP_FAILED)
		tst_brkm(TFAIL | TERRNO, cleanup, "mremap failed");

	/*
	 * Attempt to initialize the expanded memory
	 * mapped region with data. If the map area
	 * was bad, we'd get SIGSEGV.
	 */
	for (ind = 0; ind < newsize; ind++) {
		addr[ind] = (char)ind;
	}

	/*
	 * Memory mapped area is good. Now, attempt
	 * to synchronize the mapped memory region
	 * with the file.
	 */
	if (msync(addr, newsize, MS_SYNC) != 0) {
		tst_resm(TFAIL | TERRNO, "msync failed to synch "
			 "mapped file");
	} else {
		tst_resm(TPASS, "Functionality of "
			 "mremap() is correct");
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *
 * Get system page size, Set the size of virtual memory area and the
 * new size after resize,
 * Creat a temporary directory and a file under it.
 * Stratch the file size to the size of virtual memory area and
 * write 1 byte (\0). Map the temporary file for the length of virtual
 * memory (memsize) into memory.
 */
void setup(void)
{
	int pagesz;		/* system's page size */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Get the system page size */
	if ((pagesz = getpagesize()) < 0) {
		tst_brkm(TFAIL, NULL,
			 "getpagesize failed to get system page size");
	}

	/* Get the size of virtual memory area to be mapped */
	memsize = (1000 * pagesz);

	/* Get the New size of virtual memory block after resize */
	newsize = (memsize * 2);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "opening %s failed",
			 TEMPFILE);

	/* Stretch the file to the size of virtual memory area */
	if (lseek(fildes, (off_t) memsize, SEEK_SET) != (off_t) memsize) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "lseeking to %d offset pos. failed", memsize);
	}

	/* Write one byte data into temporary file */
	if (write(fildes, "\0", 1) != 1) {
		tst_brkm(TBROK, cleanup, "writing to %s failed", TEMPFILE);
	}

	/*
	 * Call mmap to map virtual memory (memsize bytes) from the
	 * beginning of temporary file (offset is 0) into memory.
	 */
	addr = mmap(0, memsize, PROT_WRITE, MAP_SHARED, fildes, 0);

	/* Check for the return value of mmap() */
	if (addr == (char *)MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmaping Failed on %s", TEMPFILE);
	}

	/* Stretch the file to newsize of virtual memory block */
	if (lseek(fildes, (off_t) newsize, SEEK_SET) != (off_t) newsize) {
		tst_brkm(TBROK, cleanup, "lseek() to %d offset pos. Failed, "
			 "error=%d : %s", newsize, errno, strerror(errno));
	}

	/* Write one byte data into temporary file */
	if (write(fildes, "\0", 1) != 1) {
		tst_brkm(TBROK | TERRNO, cleanup, "writing to %s failed",
			 TEMPFILE);
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Unmap the mapped memory area done in the test.
 *	       Close the temporary file.
 *	       Remove the temporary directory.
 */
void cleanup(void)
{

	/* Unmap the mapped memory */
	if (munmap(addr, newsize) != 0)
		tst_brkm(TBROK | TERRNO, NULL, "munmap failed");

	/* Close the temporary file */
	SAFE_CLOSE(NULL, fildes);

	tst_rmdir();
}
