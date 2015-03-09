/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Call mmap() to map a file creating mapped memory with no access under
 *  the following conditions -
 *	- The prot parameter is set to PROT_NONE
 *	- The file descriptor is open for read(any mode other than write)
 *	- The minimum file permissions should be 0444.
 *
 *  The call should succeed to map the file creating mapped memory with the
 *  required attributes.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and an attempt to access the contents of the mapped region should give
 *  rise to the signal SIGSEGV.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "test.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap05";
int TST_TOTAL = 1;

static size_t page_sz;
static volatile char *addr;
static int fildes;
static volatile int pass = 0;
static sigjmp_buf env;

static void setup(void);
static void cleanup(void);
static void sig_handler(int sig);

int main(int ac, char **av)
{
	int lc;
	char file_content;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * with no access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_NONE,
			    MAP_FILE | MAP_SHARED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap() failed on %s",
				 TEMPFILE);
			continue;
		}

		/*
		 * Try to access the mapped region.  This should
		 * generate a SIGSEGV which will be caught below.
		 *
		 * This is wrapped by the sigsetjmp() call that will
		 * take care of restoring the program's context in an
		 * elegant way in conjunction with the call to
		 * siglongjmp() in the signal handler.
		 */
		if (sigsetjmp(env, 1) == 0) {
			file_content = addr[0];
		}

		if (pass) {
			tst_resm(TPASS, "Got SIGSEGV as expected");
		} else {
			tst_resm(TFAIL, "Mapped memory region with NO "
				 "access is accessible");
		}

		/* Unmap mapped memory and reset pass in case we are looping */
		if (munmap((void *)addr, page_sz) != 0) {
			tst_brkm(TFAIL | TERRNO, cleanup, "munmap failed");
		}
		pass = 0;

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char *tst_buff;

	tst_sig(NOFORK, sig_handler, cleanup);

	TEST_PAUSE;

	page_sz = getpagesize();

	/* Allocate space for the test buffer */
	if ((tst_buff = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL, "calloc failed (tst_buff)");
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_WRONLY | O_CREAT, 0666)) < 0) {
		free(tst_buff);
		tst_brkm(TFAIL | TERRNO, cleanup, "opening %s failed",
			 TEMPFILE);
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) != page_sz) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "writing to %s failed", TEMPFILE);
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Make sure proper permissions set on file */
	if (fchmod(fildes, 0444) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "fchmod of %s failed",
			 TEMPFILE);
	}

	/* Close the temporary file opened for write */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "closing %s failed",
			 TEMPFILE);
	}

	/* Open the temporary file again for reading */
	if ((fildes = open(TEMPFILE, O_RDONLY)) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "opening %s readonly failed",
			 TEMPFILE);
	}
}

/*
 * sig_handler() - Signal Catching function.
 *   This function gets executed when the test process receives
 *   the signal SIGSEGV while trying to access the contents of memory which
 *   is not accessible.
 */
static void sig_handler(int sig)
{
	if (sig == SIGSEGV) {
		/* set the global variable and jump back */
		pass = 1;
		siglongjmp(env, 1);
	} else {
		tst_brkm(TBROK, cleanup, "received an unexpected signal: %d",
			 sig);
	}
}

static void cleanup(void)
{
	close(fildes);
	tst_rmdir();
}
