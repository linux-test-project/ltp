// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) 2026 Pavithra <pavrampu@linux.ibm.com>
 */

/*
 * Origin: https://github.com/libhugetlbfs/libhugetlbfs/blob/master/tests/task-size-overrun.c
 */

/*\
 * Test that mmap correctly rejects hugepage mappings that straddle the
 * TASK_SIZE boundary. Both MAP_FIXED (which must fail) and non-MAP_FIXED
 * (which succeeds on Linux) cases are verified. A buggy kernel would allow
 * such a mapping to succeed, violating address space limits.
 *
 * Requires root to mount hugetlbfs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "hugetlb.h"
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_macros.h"

#define MAPS_BUF_SZ 4096
#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static unsigned long task_size;
static int fd = -1;

static unsigned long find_last_mapped(void)
{
	char line[MAPS_BUF_SZ];
	char last[MAPS_BUF_SZ];
	unsigned long start, end, off, ino;
	FILE *f;
	int found = 0;

	f = SAFE_FOPEN("/proc/self/maps", "r");

	/* Read all lines and save the last non-special mapping */
	while (fgets(line, MAPS_BUF_SZ, f)) {
		/* Skip special mappings like [vsyscall], [vdso], [vvar] */
		if (strstr(line, "[vsyscall]") || strstr(line, "[vdso]") ||
		    strstr(line, "[vvar]"))
			continue;

		strncpy(last, line, MAPS_BUF_SZ - 1);
		last[MAPS_BUF_SZ - 1] = '\0';
		found = 1;
	}

	SAFE_FCLOSE(f);

	if (!found)
		tst_brk(TBROK, "Could not find any valid mapping in /proc/self/maps");

	tst_res(TINFO, "Last map: %s", last);
	if (sscanf(last, "%lx-%lx %*s %lx %*s %ld", &start, &end, &off, &ino) != 4)
		tst_brk(TBROK, "Failed to parse /proc/self/maps line");

	tst_res(TINFO, "Last map: at 0x%lx-0x%lx", start, end);
	return end;
}

static unsigned long find_task_size(void)
{
	unsigned long low, high;
	void *p;
	int page_size = getpagesize();

	low = find_last_mapped();
	if (!low || ((low % page_size) != 0))
		tst_brk(TBROK, "Bogus stack end address, 0x%lx!?", low);

	/* Convert to page frame number */
	low = low / page_size;

	/*
	 * Set high to maximum possible address space
	 * For 64-bit: (2^64 - 1) / page_size
	 * We use -1UL which gives us the maximum unsigned long value
	 */
	high = (-1UL) / page_size;

	tst_res(TINFO, "Binary searching for task size PFNs 0x%lx..0x%lx", low, high);

	while (high > low + 1) {
		unsigned long pfn = (low + high) / 2;
		unsigned long addr = pfn * page_size;

		p = mmap((void *)addr, page_size, PROT_READ,
			   MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);
		if (p == MAP_FAILED) {
			tst_res(TDEBUG | TERRNO, "Map failed at 0x%lx", addr);
			high = pfn;
		} else {
			tst_res(TDEBUG, "Map succeeded at 0x%lx", addr);
			SAFE_MUNMAP(p, page_size);
			low = pfn;
		}
	}

	return low * page_size;
}

static void run_test(void)
{
	unsigned long straddle_addr;

	straddle_addr = task_size - hpage_size;
	straddle_addr = LTP_ALIGN(straddle_addr, hpage_size);

	tst_res(TINFO, "Mapping without MAP_FIXED at %lx...", straddle_addr);

	TST_EXP_PASS_PTR_VOID(mmap((void *)straddle_addr, 2*hpage_size,
	                      PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));

	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, 2*hpage_size);

	tst_res(TINFO, "Mapping with MAP_FIXED at %lx...", straddle_addr);

	TST_EXP_FAIL_PTR_VOID(mmap((void *)straddle_addr, 2*hpage_size,
	                      PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0), ENOMEM);

	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, 2*hpage_size);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();

	task_size = find_task_size();
	tst_res(TINFO, "TASK_SIZE = 0x%lx", task_size);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {3, TST_NEEDS},
};
