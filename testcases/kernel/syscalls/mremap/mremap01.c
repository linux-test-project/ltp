// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Verify that :manpage:`mremap(2)` succeeds when used to expand an existing
 * virtual memory region that was previously mapped to a file with
 * :manpage:`mmap(2)`.
 *
 * After growing the mapping with ``MREMAP_MAYMOVE``, the expanded region must
 * be fully accessible and its contents must synchronize to the backing file
 * with :manpage:`msync(2)`.
 *
 * [Algorithm]
 *
 * - create a temporary file and stretch it to the initial mapping size
 * - map the file with ``MAP_SHARED`` and ``PROT_WRITE``
 * - stretch the file to the new (larger) size before growing the mapping
 * - grow the mapping with mremap() and ``MREMAP_MAYMOVE``
 * - write every byte of the grown region to prove it is usable
 * - synchronize the region to the backing file with ``MS_SYNC``
 * - read the data back from the backing file to prove it was synchronized
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sys/mman.h>
#include "tst_test.h"
#include "tst_safe_prw.h"

#define TEMPFILE	"mremapfile"

static char *addr = MAP_FAILED;
static size_t memsize;
static size_t newsize;
static int fildes = -1;

static void setup(void)
{
	int pagesz = getpagesize();

	memsize = 1000 * pagesz;
	newsize = memsize * 2;

	fildes = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);
}

static void verify_file(void)
{
	size_t offsets[3];
	unsigned int i;
	char got;

	offsets[0] = 0;
	offsets[1] = newsize / 2;
	offsets[2] = newsize - 1;

	for (i = 0; i < ARRAY_SIZE(offsets); i++) {
		size_t off = offsets[i];

		SAFE_PREAD(1, fildes, &got, 1, (off_t)off);

		if (got != (char)off) {
			tst_res(TFAIL, "mremap()'d region did not sync to the file. "
				"file[%zu] == 0x%02x, expected 0x%02x",
				off, (unsigned char)got, (unsigned char)off);
			return;
		}
	}

	tst_res(TPASS, "Functionality of mremap() is correct");
}

static void run(void)
{
	size_t ind;

	/*
	 * Establish a fresh memsize mapping for every iteration so mremap()
	 * always grows a newly created mapping instead of an already-grown one.
	 */
	SAFE_FTRUNCATE(fildes, 0);
	SAFE_LSEEK(fildes, (off_t)memsize, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fildes, "\0", 1);

	addr = SAFE_MMAP(0, memsize, PROT_READ | PROT_WRITE, MAP_SHARED,
			 fildes, 0);

	SAFE_LSEEK(fildes, (off_t)newsize, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fildes, "\0", 1);

	TESTPTR(mremap(addr, memsize, newsize, MREMAP_MAYMOVE));
	addr = TST_RET_PTR;
	if (addr == MAP_FAILED)
		tst_brk(TFAIL | TTERRNO, "mremap failed");

	for (ind = 0; ind < newsize; ind++)
		addr[ind] = (char)ind;

	SAFE_MSYNC(addr, newsize, MS_SYNC);

	verify_file();

	SAFE_MUNMAP(addr, newsize);
}

static void cleanup(void)
{
	if (addr != MAP_FAILED)
		SAFE_MUNMAP(addr, newsize);

	if (fildes != -1)
		SAFE_CLOSE(fildes);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_tmpdir = 1,
};
