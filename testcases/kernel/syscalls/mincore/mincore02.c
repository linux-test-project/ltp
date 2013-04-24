/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   Author: Rajeev Tiwari: rajeevti@in.ibm.com
 * Copyright (c) 2004 Gernot Payer <gpayer@suse.de>
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
 * This test case provides a functional validation for mincore system call.
 * We mmap a file of known size (multiple of page size) and lock it in
 * memory. Then we obtain page location information via mincore and compare
 * the result with the expected value.
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

char *TCID = "mincore02";
int TST_TOTAL = 1;

static int file_desc = 0;
static void *position = NULL;
static int page_size;
static int num_pages = 4;
static unsigned char *vec = NULL;

#if !defined(UCLINUX)

static char tmpfilename[] = "fooXXXXXX";

static void cleanup(void)
{
	free(vec);
	munlock(position, page_size * num_pages);
	munmap(position, page_size * num_pages);
	TEST_CLEANUP;
	close(file_desc);
	unlink(tmpfilename);
}

static void setup(void)
{
	char *buf;
	size_t size;

	page_size = getpagesize();
	if (page_size == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Unable to get page size");

	size = page_size * num_pages;
	buf = malloc(size);

	memset(buf, 42, size);
	vec = malloc((size + page_size - 1) / page_size);

	file_desc = mkstemp(tmpfilename);
	if (file_desc == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
		         "Unable to create temporary file");
	}

	/* fill the temporary file with two pages of data */
	if (write(file_desc, buf, size) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
		         "Error in writing to the file");
	}
	free(buf);

	/* mmap the file in virtual address space in read , write and execute mode , the mapping should be shared  */
	position = mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC,
	                MAP_SHARED, file_desc, 0);

	if (position == MAP_FAILED) {
		tst_brkm(TBROK | TERRNO, cleanup,
                         "Unable to map file for read/write.");
	}

	/* lock mmapped file, so mincore returns "in core" for all pages */
	if (mlock(position, size) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Unable to lock the file");
}

int main(int argc, char **argv)
{
	int lock_pages, counter;

	setup();

	if (mincore(position, num_pages * page_size, vec) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
		         "Unable to execute mincore system call");
	}

	/* check status of pages */
	lock_pages = 0;

	for (counter = 0; counter < num_pages; counter++) {
		if (vec[counter] & 1)
			lock_pages++;
	}

	if (lock_pages == num_pages)
		tst_resm(TPASS, "%d pages locked, %d pages in-core", num_pages,
			 lock_pages);
	else
		tst_resm(TFAIL,
			 "not all locked pages are in-core: no. locked: %d, no. in-core: %d",
			 num_pages, lock_pages);

	cleanup();
	tst_exit();
}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */
