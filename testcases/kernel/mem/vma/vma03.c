/*
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * This is a reproducer for CVE-2011-2496.
 *
 * The normal mmap paths all avoid creating a mapping where the pgoff
 * inside the mapping could wrap around due to overflow.  However, an
 * expanding mremap() can take such a non-wrapping mapping and make it
 * bigger and cause a wrapping condition. There is also another case
 * where we expand mappings hiding in plain sight: the automatic stack
 * expansion.
 *
 * This program tries to remap a mapping with a new size that would
 * wrap pgoff. Notice that it only works on 32-bit arch for now.
 */

#define _GNU_SOURCE
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "tst_kernel.h"
#include "lapi/abisize.h"

char *TCID = "vma03";
int TST_TOTAL = 1;

#ifdef __NR_mmap2
#define TESTFILE "testfile"

static size_t pgsz;
static int fd;

static void *mmap2(void *addr, size_t length, int prot,
		   int flags, int fd, off_t pgoffset);
static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	void *map, *remap;
	off_t pgoff;

	if (TST_ABI != 32 || tst_kernel_bits() != 32) {
		tst_brkm(TCONF, NULL,
			 "test is designed for 32-bit system only.");
	}

	tst_parse_opts(argc, argv, NULL, NULL);

	pgsz = sysconf(_SC_PAGE_SIZE);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		fd = SAFE_OPEN(NULL, TESTFILE, O_RDWR);

		/*
		 * The pgoff is counted in 4K units and must be page-aligned,
		 * hence we must align it down to page_size/4096 in a case that
		 * the system has page_size > 4K.
		 */
		pgoff = (ULONG_MAX - 1)&(~((pgsz-1)>>12));
		map = mmap2(NULL, pgsz, PROT_READ | PROT_WRITE, MAP_PRIVATE,
			    fd, pgoff);
		if (map == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup, "mmap2");

		remap = mremap(map, pgsz, 2 * pgsz, 0);
		if (remap == MAP_FAILED) {
			if (errno == EINVAL)
				tst_resm(TPASS, "mremap failed as expected.");
			else
				tst_resm(TFAIL | TERRNO, "mremap");
			munmap(map, pgsz);
		} else {
			tst_resm(TFAIL, "mremap succeeded unexpectedly.");
			munmap(remap, 2 * pgsz);
		}

		close(fd);
	}

	cleanup();
	tst_exit();
}

static void *mmap2(void *addr, size_t length, int prot,
		   int flags, int fd, off_t pgoffset)
{
	return (void *)syscall(SYS_mmap2, addr, length, prot,
			       flags, fd, pgoffset);
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	fd = SAFE_CREAT(NULL, TESTFILE, 0644);
	close(fd);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
#else /* __NR_mmap2 */
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "__NR_mmap2 is not defined on your system");
}
#endif
