// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 SUSE LLC
 * Author: Vlastimil Babka <vbabka@suse.cz>
 * https://bugzilla.suse.com/attachment.cgi?id=867254
 * LTP port: Petr Vorel <pvorel@suse.cz>
 */

/*\
 * [Description]
 *
 * Bug reproducer for 7e7757876f25 ("mm/mremap: fix vm_pgoff in vma_merge() case 3")
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <lapi/mmap.h>

#include "tst_test.h"
#include "tst_safe_macros.h"

#define NUM_GRANULARITYS 3

static int fd;
static char *buf, *buf2;
static int mmap_size, mremap_size;

static struct tcase {
	size_t incompatible;
	const char *desc;
} tcases[] = {
	{
		.desc = "all pages with compatible mapping",
	},
	{
		.incompatible = 3,
		.desc = "third MMAP_GRANULARITY's mapping incompatible",
	},
	{
		.incompatible = 1,
		.desc = "first MMAP_GRANULARITY's mapping incompatible",
	},
};

static int check_pages(void)
{
	int fail = 0, i;
	char val;

	for (i = 0; i < (int)ARRAY_SIZE(tcases); i++) {
		val = buf[i * MMAP_GRANULARITY];
		if (val != 0x30 + i) {
			tst_res(TFAIL, "page %d wrong value %d (0x%x)", i, val - 0x30, val);
			fail = 1;
		}
	}

	return fail;
}

static void do_test(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int ret;

	tst_res(TINFO, "%s", tc->desc);

	buf = SAFE_MMAP(0, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	buf2 = mremap(buf + MMAP_GRANULARITY, MMAP_GRANULARITY, MMAP_GRANULARITY,
			MREMAP_MAYMOVE|MREMAP_FIXED, buf + mremap_size);
	if (buf2 == MAP_FAILED)
		tst_brk(TBROK, "mremap() failed");

	if (tc->incompatible) {
		ret = mprotect(buf + (tc->incompatible-1)*MMAP_GRANULARITY,
					MMAP_GRANULARITY, PROT_READ);
		if (ret == -1)
			tst_brk(TBROK, "mprotect() failed");
	}

	buf2 = mremap(buf + mremap_size, MMAP_GRANULARITY, MMAP_GRANULARITY,
			MREMAP_MAYMOVE|MREMAP_FIXED, buf + MMAP_GRANULARITY);
	if (buf2 == MAP_FAILED)
		tst_brk(TBROK, "mremap() failed");

	if (!check_pages())
		tst_res(TPASS, "mmap/mremap work properly");

	SAFE_MUNMAP(buf, mremap_size);
}

static void setup(void)
{
	int ret, i;

	mmap_size = (NUM_GRANULARITYS+1) * MMAP_GRANULARITY;
	mremap_size = NUM_GRANULARITYS * MMAP_GRANULARITY;

	fd = SAFE_OPEN("testfile", O_CREAT | O_RDWR | O_TRUNC, 0600);

	ret = fallocate(fd, 0, 0, mmap_size);
	if (ret != 0) {
		if (tst_fs_type(".") == TST_NFS_MAGIC && (errno == EOPNOTSUPP ||
							  errno == ENOSYS)) {
			tst_brk(TCONF, "fallocate system call is not implemented");
		}
		tst_brk(TBROK, "fallocate() failed");
	}

	buf = SAFE_MMAP(0, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	for (i = 0; i < (int)ARRAY_SIZE(tcases)+1; i++)
		buf[i*MMAP_GRANULARITY] = 0x30 + i;

	/* clear the page tables */
	SAFE_MUNMAP(buf, mmap_size);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.tags = (struct tst_tag[]) {
		{"linux-git", "7e7757876f25"},
		{}
	},
};
