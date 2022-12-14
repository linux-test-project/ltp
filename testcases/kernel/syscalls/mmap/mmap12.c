// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 FNST, DAN LI <li.dan@cn.fujitsu.com>
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

#include "tst_test.h"

#define TEMPFILE        "mmapfile"
#define PATHLEN         256
#define MMAPSIZE        (1UL<<20)

static int fildes;
static char *addr;

static void page_check(void)
{
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
			tst_res(TCONF | TERRNO,
				"don't have permission to open dev pagemap");
			return;
		} else {
			tst_brk(TFAIL | TERRNO, "pen dev pagemap failed");
		}
	}

	offset = SAFE_LSEEK(pm, index, SEEK_SET);
	if (offset != index)
		tst_brk(TFAIL | TERRNO, "Reposition offset failed");

	while (i <= num_pages) {
		SAFE_READ(1, pm, &pagemap, sizeof(uint64_t));

		/*
		 * Check if the page is present.
		 */
		if (!(pagemap & (1ULL<<63))) {
			tst_res(TINFO, "The %dth page addressed at %lX is not "
				       "present", i, vmstart + i * page_sz);
			flag = 1;
		}

		i++;
	}

	close(pm);

	if (!flag)
		tst_res(TINFO, "All pages are present");
}

void verify_mmap(void)
{
	unsigned int i;

	addr = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_POPULATE, fildes, 0);

	if (addr == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "mmap of %s failed", TEMPFILE);
		return;
	}

	page_check();

	for (i = 0; i < MMAPSIZE; i++) {
		if (addr[i]) {
			tst_res(TFAIL, "Non-zero byte at offset %i", i);
			goto unmap;
		}
	}

	tst_res(TPASS, "File mapped properly");

unmap:
	SAFE_MUNMAP(addr, MMAPSIZE);
}

static void setup(void)
{
	fildes = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0766);

	SAFE_FTRUNCATE(fildes, MMAPSIZE);
}

static void cleanup(void)
{
	if (fildes > 0)
		SAFE_CLOSE(fildes);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_mmap,
	.needs_tmpdir = 1,
};
