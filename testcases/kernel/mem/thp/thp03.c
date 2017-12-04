/*
 * Copyright (C) 2012-2017  Red Hat, Inc.
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
 */

/* thp03 - Case for spliting unaligned memory.
 *       - System will panic if failed.
 *
 * Modified form a reproducer for
 *          https://patchwork.kernel.org/patch/1358441/
 * Kernel Commit id: 027ef6c87853b0a9df53175063028edb4950d476
 * There was a bug in THP, will crash happened due to the following
 * reason according to developers:
 *
 * most VM places are using pmd_none but a few are still using
 * pmd_present. The meaning is about the same for the pmd. However
 * pmd_present would return the wrong value on PROT_NONE ranges or in
 * case of a non reproducible race with split_huge_page.
 * When the code using pmd_present gets a false negative, the kernel will
 * crash. It's just an annoying DoS with a BUG_ON triggering: no memory
 * corruption and no data corruption (nor userland nor kernel).
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mem.h"
#include "lapi/mmap.h"

static void thp_test(void);

static long hugepage_size;
static long unaligned_size;
static long page_size;

static void thp_test(void)
{
	void *p;

	p = SAFE_MMAP(NULL, unaligned_size, PROT_READ | PROT_WRITE,
		 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	memset(p, 0x00, unaligned_size);
	if (mprotect(p, unaligned_size, PROT_NONE) == -1)
		tst_brk(TBROK | TERRNO, "mprotect");

	if (madvise(p + hugepage_size, page_size, MADV_MERGEABLE) == -1) {
		if (errno == EINVAL) {
			tst_brk(TCONF,
			         "MADV_MERGEABLE is not enabled/supported");
		} else {
			tst_brk(TBROK | TERRNO, "madvise");
		}
	}

	switch (SAFE_FORK()) {
	case 0:
		exit(0);
	default:
		SAFE_WAITPID(-1, NULL, 0);
	}

	tst_res(TPASS, "system didn't crash, pass.");
	munmap(p, unaligned_size);
}

static void setup(void)
{
	if (access(PATH_THP, F_OK) == -1)
		tst_brk(TCONF, "THP not enabled in kernel?");

	check_hugepage();

	hugepage_size = SAFE_READ_MEMINFO("Hugepagesize:") * KB;
	unaligned_size = hugepage_size * 4 - 1;
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test_all = thp_test,
};
