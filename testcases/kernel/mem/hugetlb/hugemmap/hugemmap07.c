// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * Certain kernels have a bug where brk() does not perform the same
 * checks that a MAP_FIXED mmap() will, allowing brk() to create a
 * normal page VMA in a hugepage only address region. This can lead
 * to oopses or other badness.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"
#include "tst_safe_stdio.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int huge_fd = -1;

#ifdef __powerpc64__
static int arch_has_slice_support(void)
{
	char mmu_type[16];

	SAFE_FILE_LINES_SCANF("/proc/cpuinfo", "MMU : %16s", mmu_type);
	return strcmp(mmu_type, "Hash") == 0;
}

static void *next_chunk(void *addr)
{
	if (!arch_has_slice_support())
		return PALIGN(addr, hpage_size);

	if ((unsigned long)addr < 0x100000000UL)
		/* 256M segments below 4G */
		return PALIGN(addr, 0x10000000UL);
	/* 1TB segments above */
	return PALIGN(addr, 0x10000000000UL);
}
#elif defined(__powerpc__)
static void *next_chunk(void *addr)
{
	if (tst_kernel_bits() == 32)
		return PALIGN(addr, hpage_size);
	else
		return PALIGN(addr, 0x10000000UL);
}
#elif defined(__ia64__)
static void *next_chunk(void *addr)
{
	return PALIGN(addr, 0x8000000000000000UL);
}
#else
static void *next_chunk(void *addr)
{
	return PALIGN(addr, hpage_size);
}
#endif

static void run_test(void)
{
	void *brk0, *hugemap_addr, *newbrk;
	char *p;
	int err;

	brk0 = sbrk(0);
	tst_res(TINFO, "Initial break at %p", brk0);

	hugemap_addr = next_chunk(brk0) + hpage_size;

	p = SAFE_MMAP(hugemap_addr, hpage_size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE|MAP_FIXED, huge_fd, 0);
	if (p != hugemap_addr) {
		tst_res(TFAIL, "mmap() at unexpected address %p instead of %p\n", p,
		     hugemap_addr);
		goto cleanup;
	}

	newbrk = next_chunk(brk0) + getpagesize();
	err = brk((void *)newbrk);
	if (err == -1) {
		/* Failing the brk() is an acceptable kernel response */
		tst_res(TPASS, "Failing the brk at %p is an acceptable response",
				newbrk);
	} else {
		/* Suceeding the brk() is acceptable if the new memory is
		 * properly accesible and we don't have a kernel blow up when
		 * we touch it.
		 */
		tst_res(TINFO, "New break at %p", newbrk);
		memset(brk0, 0, newbrk-brk0);
		tst_res(TPASS, "memory is accessible, hence successful brk() is "
				"an acceptable response");
	}
cleanup:
	SAFE_MUNMAP(p, hpage_size);
	err = brk(brk0);
	if (err == -1)
		tst_brk(TBROK, "Failed to set break at the original position");
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
	huge_fd = tst_creat_unlinked(MNTPOINT, 0);
}

static void cleanup(void)
{
	SAFE_CLOSE(huge_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.taint_check = TST_TAINT_D | TST_TAINT_W,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {1, TST_NEEDS},
};
