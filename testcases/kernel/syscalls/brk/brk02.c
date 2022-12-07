// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Liam R. Howlett <liam.howlett@oracle.com>
 */

/*\
 * [Description]
 *
 * Expand brk() by at least 2 pages to ensure there is a newly created VMA
 * and not expanding the original due to multiple anon pages.  mprotect() that
 * new VMA then brk() back to the original address therefore causing a munmap of
 * at least one full VMA.
 */

#include <unistd.h>
#include <sys/mman.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static void *brk_variants(void *addr)
{
	void *brk_addr;

	if (tst_variant) {
		brk_addr = (void *)tst_syscall(__NR_brk, addr);
	} else {
		TST_EXP_PASS_SILENT(brk(addr), "brk()");
		brk_addr = (void *)sbrk(0);
	}

	return brk_addr;
}

static void brk_down_vmas(void)
{
	void *brk_addr;

	if (tst_variant) {
		tst_res(TINFO, "Testing syscall variant");
		brk_addr = (void *)tst_syscall(__NR_brk, 0);
	} else {
		tst_res(TINFO, "Testing libc variant");
		brk_addr = (void *)sbrk(0);

		if (brk_addr == (void *)-1)
			tst_brk(TCONF, "sbrk() not implemented");

		/*
		 * Check if brk itself is implemented: updating to the current break
		 * should be a no-op.
		 */
		if (brk(brk_addr) != 0)
			tst_brk(TCONF, "brk() not implemented");
	}

	unsigned long page_size = getpagesize();
	void *addr = brk_addr + page_size;

	if (brk_variants(addr) < addr) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() by page size");
		return;
	}

	addr += page_size;
	if (brk_variants(addr) < addr) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() by 2x page size");
		return;
	}

	if (mprotect(addr - page_size, page_size, PROT_READ)) {
		tst_res(TFAIL | TERRNO, "Cannot mprotect new VMA");
		return;
	}

	addr += page_size;
	if (brk_variants(addr) < addr) {
		tst_res(TFAIL | TERRNO, "Cannot expand brk() after mprotect");
		return;
	}

	if (brk_variants(brk_addr) != brk_addr) {
		tst_res(TFAIL | TERRNO, "Cannot restore brk() to start address");
		return;
	}

	tst_res(TPASS, "munmap at least two VMAs of brk() passed");
}

static struct tst_test test = {
	.test_all = brk_down_vmas,
	.test_variants = 2,
};
