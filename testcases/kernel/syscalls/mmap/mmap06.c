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
 *  Call mmap() to map a file creating a mapped region with read access
 *  under the following conditions -
 *	- The prot parameter is set to PROT_READ
 *	- The file descriptor is open for writing.
 *
 *  The call should fail to map the file.
 *
 * Expected Result:
 *  mmap() should fail returning -1 and errno should get set to EACCES.
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

char *TCID = "mmap06";
int TST_TOTAL = 1;

static size_t page_sz;
static char *addr;
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
		 * with read access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_READ,
			    MAP_FILE | MAP_SHARED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL | TERRNO,
				 "mmap() returned invalid value, expected: %p",
				 MAP_FAILED);
			/* Unmap the mapped memory */
			if (munmap(addr, page_sz) != 0) {
				tst_resm(TBROK, "munmap() failed");
				cleanup();
			}
			continue;
		}
		if (TEST_ERRNO == EACCES) {
			tst_resm(TPASS, "mmap failed with EACCES");
		} else {
			tst_resm(TFAIL | TERRNO,
				 "mmap failed with unexpected errno");
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

	/* Allocate space for the test buffer */
	if ((tst_buff = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL, "calloc() failed (tst_buff)");
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

	free(tst_buff);
}

static void cleanup(void)
{
	close(fildes);
	tst_rmdir();
}
