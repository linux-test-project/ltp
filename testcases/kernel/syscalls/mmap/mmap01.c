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
 * Test Name: mmap01
 *
 * Test Description:
 *  Verify that, mmap() succeeds when used to map a file where size of the
 *  file is not a multiple of the page size, the memory area beyond the end
 *  of the file to the end of the page is accessible. Also, verify that
 *  this area is all zeroed and the modifications done to this area are
 *  not written to the file.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region.
 *  The memory area beyond the end of file to the end of page should be
 *  filled with zero.
 *  The changes beyond the end of file should not get written to the file.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  mmap01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
char *addr;			/* addr of memory mapped region */
char *dummy;			/* dummy string */
size_t page_sz;			/* system page size */
size_t file_sz;			/* mapped file size */
int fildes;			/* file descriptor for tempfile */
char Cmd_buffer[BUFSIZ];	/* command buffer to hold test command */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call mmap to map the temporary file beyond EOF
		 * with write access.
		 */
		errno = 0;
		addr = mmap(addr, page_sz, PROT_READ | PROT_WRITE,
			    MAP_FILE | MAP_SHARED | MAP_FIXED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL, "mmap() Failed on %s, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Check if mapped memory area beyond EOF are
			 * zeros and changes beyond EOF are not written
			 * to file.
			 */
			if (memcmp(&addr[file_sz], dummy, page_sz - file_sz)) {
				tst_brkm(TFAIL, cleanup, "mapped memory area "
					 "contains invalid data");
			}

			/*
			 * Initialize memory beyond file size
			 */
			addr[file_sz] = 'X';
			addr[file_sz + 1] = 'Y';
			addr[file_sz + 2] = 'Z';

			/*
			 * Synchronize the mapped memory region
			 * with the file.
			 */
			if (msync(addr, page_sz, MS_SYNC) != 0) {
				tst_brkm(TFAIL, cleanup, "msync() failed to "
					 "synchronize mapped file, error:%d",
					 errno);
			}

			/*
			 * Now, Search for the pattern 'XYZ' in the
			 * temporary file.  The pattern should not be
			 * found and the return value should be 1.
			 */
			if (system(Cmd_buffer) != 0) {
				tst_resm(TPASS,
					 "Functionality of mmap() successful");
			} else {
				tst_resm(TFAIL,
					 "Specified pattern found in file");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
		if (munmap(addr, page_sz) != 0) {
			tst_brkm(TFAIL, NULL, "munmap() fails to unmap the "
				 "memory, errno=%d", errno);
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *	     Get system page size, allocate and initialize the string dummy.
 *	     Initialize addr such that it is more than one page below the break
 *	     address of the process, and initialize one page region from addr
 *	     with char 'A'.
 *	     Creat a temporary directory and a file under it.
 *	     Write some known data into file and get the size of the file.
 */
void setup()
{
	struct stat stat_buf;
	char Path_name[PATH_MAX];	/* pathname of temporary file */
	char write_buf[] = "hello world\n";

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Get the path of temporary file to be created */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "getcwd fails to get current working directory");
	}

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TFAIL, cleanup,
			 "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Write some data into temporary file */
	if (write(fildes, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TFAIL, cleanup,
			 "write() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Get the size of temporary file */
	if (stat(TEMPFILE, &stat_buf) < 0) {
		tst_brkm(TFAIL, cleanup,
			 "stat() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}
	file_sz = stat_buf.st_size;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, cleanup,
			 "getpagesize() fails to get system page size");
		tst_exit();
	}

	/* Allocate and initialize dummy string of system page size bytes */
	if ((dummy = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, cleanup, "calloc() failed to allocate space");
		tst_exit();
	}

	/*
	 * Initialize addr to align with the first segment boundary address
	 * above the break address of the process.
	 */
	addr = (void *)(((intptr_t) sbrk(0) + (SHMLBA - 1)) & ~(SHMLBA - 1));

	/* Set the break address of the process to the addr plus one
	 * page size.
	 */
	if ((intptr_t) sbrk(SHMLBA + page_sz) == -1) {
		tst_brkm(TFAIL, cleanup, "sbrk(SHMLBA + page_sz) failed");
	}

	/* Initialize one page region from addr with 'A' */
	memset(addr, 'A', page_sz);

	/* Create the command which will be executed in the test */
	sprintf(Cmd_buffer, "grep XYZ %s/%s > /dev/null", Path_name, TEMPFILE);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Free the memory allocated to dummy variable.
 *	       Remove the temporary directory created.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	close(fildes);

	TEST_CLEANUP;

	/* Free the memory allocated for dummy string */
	if (dummy) {
		free(dummy);
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
