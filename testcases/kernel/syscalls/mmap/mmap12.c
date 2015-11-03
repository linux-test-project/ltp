/*
 * Copyright (c) 2013 FNST, DAN LI <li.dan@cn.fujitsu.com>
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
 *  Verify MAP_POPULATE works fine.
 *  "For a file mapping, this causes read-ahead on the file.
 *   Later accesses to the mapping will not be blocked by page faults"
 *
 * Expected Result:
 *  mmap() with MAP_POPULATE should succeed returning the address of the
 *  mapped region and this file has been read into RAM, so pages should
 *  be present.
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

#define TEMPFILE        "mmapfile"
#define PATHLEN         256
#define MMAPSIZE        (1UL<<20)

char *TCID = "mmap12";
int TST_TOTAL = 1;

static int fildes;
static char *addr;

static int page_check(void);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		addr = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_POPULATE, fildes, 0);

		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap of %s failed", TEMPFILE);
			continue;
		}

		if (page_check())
			tst_resm(TFAIL, "Not all pages are present");
		else
			tst_resm(TPASS, "Functionality of mmap() "
						"successful");
		if (munmap(addr, MMAPSIZE) != 0)
			tst_brkm(TFAIL | TERRNO, NULL, "munmap failed");
	}

	cleanup();
	tst_exit();
}

static int page_check(void)
{
	int ret;
	int i = 1;
	int flag = 0;
	int pm;
	int num_pages;
	long index;
	off_t offset;
	size_t page_sz;
	uint64_t pagemap;
	unsigned long vmstart;

	vmstart = (unsigned long)addr;
	page_sz = getpagesize();

	num_pages = MMAPSIZE / page_sz;
	index = (vmstart / page_sz) * sizeof(uint64_t);

	pm = open("/proc/self/pagemap", O_RDONLY);
	if (pm == -1) {
		if ((errno == EPERM) && (geteuid() != 0)) {
			tst_brkm(TCONF | TERRNO, NULL,
				"don't have permission to open dev pagemap");
		} else {
			tst_brkm(TFAIL | TERRNO, NULL,
				"Open dev pagemap failed");
		}
	}

	offset = lseek(pm, index, SEEK_SET);
	if (offset != index)
		tst_brkm(TFAIL | TERRNO, NULL, "Reposition offset failed");

	while (i <= num_pages) {
		ret = read(pm, &pagemap, sizeof(uint64_t));
		if (ret < 0)
			tst_brkm(TFAIL | TERRNO, NULL, "Read pagemap failed");
		/*
		 * Check if the page is present.
		 */
		if (!(pagemap & (1ULL<<63))) {
			tst_resm(TINFO, "The %dth page addressed at %lX is not "
					"present", i, vmstart + i * page_sz);
			flag = 1;
		}

		i++;
	}

	close(pm);

	if (flag)
		return 1;

	return 0;
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	if ((tst_kvercmp(2, 6, 25)) < 0)
		tst_brkm(TCONF, NULL,
			"This test can only run on kernels that are 2.6.25 and "
			"higher");

	TEST_PAUSE;

	tst_tmpdir();

	fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0766);
	if (fildes < 0)
		tst_brkm(TFAIL, cleanup, "opening %s failed", TEMPFILE);

	if (ftruncate(fildes, MMAPSIZE) < 0)
		tst_brkm(TFAIL | TERRNO, cleanup, "ftruncate file failed");

}

static void cleanup(void)
{
	close(fildes);
	tst_rmdir();
}
