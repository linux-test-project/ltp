// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 FNST, DAN LI <li.dan@cn.fujitsu.com>
 * Copyright (c) 2024 Ricardo B. Marliere <rbm@suse.com>
 */

/*\
 * Verify basic functionality of mmap(2) with MAP_LOCKED.
 *
 * mmap(2) should succeed returning the address of the mapped region,
 * and this region should be locked into memory.
 */
#include <stdio.h>
#include <sys/mman.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define TEMPFILE        "mmapfile"
#define MMAPSIZE        (1UL<<20)
#define LINELEN         256

static char *addr;

static void getvmlck(unsigned int *lock_sz)
{
	char line[LINELEN];
	FILE *fstatus = NULL;

	fstatus = SAFE_FOPEN("/proc/self/status", "r");

	while (fgets(line, LINELEN, fstatus) != NULL)
		if (strstr(line, "VmLck") != NULL)
			break;

	SAFE_SSCANF(line, "%*[^0-9]%d%*[^0-9]", lock_sz);

	SAFE_FCLOSE(fstatus);
}

static void run(void)
{
	unsigned int sz_before;
	unsigned int sz_after;
	unsigned int sz_ch;

	getvmlck(&sz_before);

	addr = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_LOCKED | MAP_ANONYMOUS, -1, 0);

	if (addr == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "mmap() of %s failed", TEMPFILE);
		return;
	}

	getvmlck(&sz_after);

	sz_ch = sz_after - sz_before;
	if (sz_ch == MMAPSIZE / 1024) {
		tst_res(TPASS, "mmap() locked %uK", sz_ch);
	} else {
		tst_res(TFAIL, "Expected %luK locked, get %uK locked",
				MMAPSIZE / 1024, sz_ch);
	}

	SAFE_MUNMAP(addr, MMAPSIZE);
}

static struct tst_test test = {
	.needs_root = 1,
	.test_all = run,
};
