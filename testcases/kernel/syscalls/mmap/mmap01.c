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
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "test.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap01";
int TST_TOTAL = 1;

static char *addr;
static char *dummy;
static size_t page_sz;
static size_t file_sz;
static int fildes;
static char cmd_buffer[BUFSIZ];

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
		 * Call mmap to map the temporary file beyond EOF
		 * with write access.
		 */
		errno = 0;
		addr = mmap(NULL, page_sz, PROT_READ | PROT_WRITE,
			    MAP_FILE | MAP_SHARED, fildes, 0);

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap of %s failed", TEMPFILE);
			continue;
		}

		/*
		 * Check if mapped memory area beyond EOF are
		 * zeros and changes beyond EOF are not written
		 * to file.
		 */
		if (memcmp(&addr[file_sz], dummy, page_sz - file_sz)) {
			tst_brkm(TFAIL, cleanup,
				 "mapped memory area contains invalid "
				 "data");
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
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "failed to synchronize mapped file");
		}

		/*
		 * Now, Search for the pattern 'XYZ' in the
		 * temporary file.  The pattern should not be
		 * found and the return value should be 1.
		 */
		if (system(cmd_buffer) != 0) {
			tst_resm(TPASS,
				 "Functionality of mmap() successful");
		} else {
			tst_resm(TFAIL,
				 "Specified pattern found in file");
		}

		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
		if (munmap(addr, page_sz) != 0) {
			tst_brkm(TFAIL | TERRNO, NULL, "munmap failed");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct stat stat_buf;
	char Path_name[PATH_MAX];
	char write_buf[] = "hello world\n";

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get the path of temporary file to be created */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			 "getcwd failed to get current working directory");
	}

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TFAIL, cleanup, "opening %s failed", TEMPFILE);
	}

	/* Write some data into temporary file */
	if (write(fildes, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TFAIL, cleanup, "writing to %s", TEMPFILE);
	}

	/* Get the size of temporary file */
	if (stat(TEMPFILE, &stat_buf) < 0) {
		tst_brkm(TFAIL | TERRNO, cleanup, "stat of %s failed",
			 TEMPFILE);
	}
	file_sz = stat_buf.st_size;

	page_sz = getpagesize();

	/* Allocate and initialize dummy string of system page size bytes */
	if ((dummy = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, cleanup, "calloc failed (dummy)");
	}

	/* Create the command which will be executed in the test */
	sprintf(cmd_buffer, "grep XYZ %s/%s > /dev/null", Path_name, TEMPFILE);
}

static void cleanup(void)
{
	close(fildes);
	free(dummy);
	tst_rmdir();
}
