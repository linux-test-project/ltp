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
 *  Call mmap() to map a file creating a mapped region with read/exec access
 *  under the following conditions -
 *	- The prot parameter is set to PROT_READ|PROT_EXEC
 *	- The file descriptor is open for read
 *	- The file being mapped has read and execute permission bit set.
 *	- The minimum file permissions should be 0555.
 *
 *  The call should succeed to map the file creating mapped memory with the
 *  required attributes.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and the mapped region should contain the contents of the mapped file.
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

#include "test.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap04";
int TST_TOTAL = 1;

static size_t page_sz;
static char *addr;
static char *dummy;
static int fildes;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * with read and execute access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_READ | PROT_EXEC,
			    MAP_FILE | MAP_SHARED, fildes, 0);

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap of %s failed", TEMPFILE);
			continue;
		}

		/*
		 * Read the file contents into the dummy
		 * variable.
		 */
		if (read(fildes, dummy, page_sz) < 0) {
			tst_brkm(TFAIL, cleanup, "reading %s failed",
				 TEMPFILE);
		}

		/*
		 * Check whether the mapped memory region
		 * has the file contents.
		 */
		if (memcmp(dummy, addr, page_sz)) {
			tst_resm(TFAIL,
				 "mapped memory region contains invalid "
				 "data");
		} else {
			tst_resm(TPASS,
				 "Functionality of mmap() successful");
		}

		/* Clean up things in case we are looping. */
		/* Unmap the mapped memory */
		if (munmap(addr, page_sz) != 0) {
			tst_brkm(TFAIL, cleanup, "munmapping failed");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char *tst_buff;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	page_sz = getpagesize();

	if ((tst_buff = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL, "calloc failed (tst_buff)");
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_WRONLY | O_CREAT, 0666)) < 0) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "opening %s failed", TEMPFILE);
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) < page_sz) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "writing to %s failed", TEMPFILE);
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Make sure proper permissions set on file */
	if (fchmod(fildes, 0555) < 0) {
		tst_brkm(TFAIL, cleanup, "fchmod of %s failed", TEMPFILE);
	}

	/* Close the temporary file opened for write */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, cleanup, "closing %s failed", TEMPFILE);
	}

	/* Allocate and initialize dummy string of system page size bytes */
	if ((dummy = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, cleanup, "calloc failed (dummy)");
	}

	/* Open the temporary file again for reading */
	if ((fildes = open(TEMPFILE, O_RDONLY)) < 0) {
		tst_brkm(TFAIL, cleanup,
			 "opening %s read-only failed", TEMPFILE);
	}
}

static void cleanup(void)
{
	close(fildes);
	free(dummy);
	tst_rmdir();
}
