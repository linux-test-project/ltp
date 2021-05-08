// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Liam R. Howlett <liam.howlett@oracle.com>
 */

/*\
 * [Description]
 * Expand brk() by at least 2 pages to ensure there is a newly created VMA
 * and not expanding the original due to multiple anon pages.  mprotect() that
 * new VMA then brk() back to the original address therefore causing a munmap of
 * at least one full VMA.
 */

#include <unistd.h>
#include <sys/mman.h>
#include "tst_test.h"

void brk_down_vmas(void)
{
	void *brk_addr = sbrk(0);

	if (brk_addr == (void *) -1)
		tst_brk(TBROK, "sbrk() failed");

	unsigned long page_size = getpagesize();
	void *addr = brk_addr + page_size;

	if (brk(addr)) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() by page size");
		return;
	}

	addr += page_size;
	if (brk(addr)) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() by 2x page size");
		return;
	}

	if (mprotect(addr - page_size, page_size, PROT_READ)) {
		tst_res(TFAIL | TERRNO, "Cannot mprotect new VMA");
		return;
	}

	addr += page_size;
	if (brk(addr)) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() after mprotect");
		return;
	}

	if (brk(brk_addr)) {
		tst_res(TFAIL | TERRNO, "Cannot restore brk() to start address");
		return;
	}

	tst_res(TPASS, "munmap at least two VMAs of brk() passed");
}

static struct tst_test test = {
	.test_all = brk_down_vmas,
};
