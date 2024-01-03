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
 * Test Name: munmap01
 *
 * Test Description:
 *  Verify that, munmap call will succeed to unmap a mapped file or
 *  anonymous shared memory region from the calling process's address space
 *  and after successful completion of munmap, the unmapped region is no
 *  longer accessible.
 *
 * Expected Result:
 *  munmap call should succeed to unmap a mapped file or anonymous shared
 *  memory region from the process's address space and it returns with a
 *  value 0, further reference to the unmapped region should result in a
 *  segmentation fault (SIGSEGV).
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
 *  munmap01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  None.$
 */
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "test.h"
#include "safe_macros.h"

#define TEMPFILE	"mmapfile"

char *TCID = "munmap01";
int TST_TOTAL = 1;

char *addr;			/* addr of memory mapped region */
int fildes;			/* file descriptor for tempfile */
unsigned int map_len;		/* length of the region to be mapped */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal catching function */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		setup();

		/*
		 * Call munmap to unmap the mapped region of the
		 * temporary file from the calling process's address space.
		 */
		TEST(munmap(addr, map_len));

		/* Check for the return value of munmap() */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "munmap() fails, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Check whether further reference is possible
		 * to the unmapped memory region by writing
		 * to the first byte of region with
		 * some arbitrary number.
		 */
		*addr = 50;

		/* This message is printed if no SIGSEGV */
		tst_resm(TFAIL, "process succeeds to refer unmapped memory region");

		cleanup();

	}
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 * Set up signal handler to catch SIGSEGV.
 * Get system page size, create a temporary file for reading/writing,
 * write one byte data into it, map the open file for the specified
 * map length.
 */
void setup(void)
{
	size_t page_sz;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* call signal function to trap the signal generated */
	if (signal(SIGSEGV, sig_handler) == SIG_ERR) {
		tst_brkm(TBROK, cleanup, "signal fails to catch signal");
	}

	TEST_PAUSE;

	/* Get the system page size */
	page_sz = getpagesize();

	/*
	 * Get the length of the open file to be mapped into process
	 * address space.
	 */
	map_len = 3 * page_sz;

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/*
	 * move the file pointer to maplength position from the beginning
	 * of the file.
	 */
	SAFE_LSEEK(cleanup, fildes, map_len, SEEK_SET);

	/* Write one byte into temporary file */
	if (write(fildes, "a", 1) != 1) {
		tst_brkm(TBROK, cleanup, "write() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/*
	 * map the open file 'TEMPFILE' from its beginning up to the maplength
	 * into the calling process's address space at the system choosen
	 * with read/write permissions to the mapped region.
	 */
	addr = mmap(0, map_len, PROT_READ | PROT_WRITE,
		    MAP_FILE | MAP_SHARED, fildes, 0);

	/* check for the return value of mmap system call */
	if (addr == (char *)MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap() Failed on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

}

/*
 * sig_handler() - signal catching function.
 *   This function is used to trap the signal generated when tried to read or
 *   write to the memory mapped region which is already detached from the
 *   calling process address space.
 *   this function is invoked when SIGSEGV generated and it calls test
 *   cleanup function and exit the program.
 */
void sig_handler(void)
{
	tst_resm(TPASS, "Functionality of munmap() successful");

	/* Invoke test cleanup function and exit */
	cleanup();

	tst_exit();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  	       Close the temporary file.
 *  	       Remove the temporary directory.
 */
void cleanup(void)
{

	/* Close the temporary file */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, NULL, "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	tst_rmdir();
}
