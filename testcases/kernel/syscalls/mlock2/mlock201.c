// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * Check the basic functionality of the mlock2(2) since kernel v2.6.9:
 * 1) When we use mlock2() without MLOCK_ONFAULT to lock memory in the
 *    specified range that is multiples of page size or not, we can
 *    show correct size of locked memory by VmLck from /proc/PID/status
 *    and lock all pages including non-present.
 * 2) When we use mlock2() with MLOCK_ONFAULT to lock memory in the
 *    specified range that is multiples of page size or not, we can
 *    show correct size of locked memory by VmLck from /proc/PID/status
 *    and just lock present pages.
 */
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/mlock2.h"

#define PAGES	8
#define HPAGES	(PAGES / 2)

static size_t pgsz;
static unsigned char vec[PAGES+1];

static struct tcase {
	size_t populate_pages;
	size_t lock_pages;
	size_t offset;
	size_t exp_vmlcks;
	size_t exp_present_pages;
	int flag;
} tcases[] = {
	/* lock single page, expect it to be locked and present */
	{0, 1, 0, 1, 1, 0},

	/* lock all pages, expect all to be locked and present */
	{0, PAGES, 0, PAGES, PAGES, 0},

	/* mlock2() locks 3 pages if the specified
	 * range is little more than 2 pages.
	 */
	{0, 2, 1, 3, 3, 0},

	/* mlock2() locks 2 pages if the specified
	 * range is little less than 2 pages.
	 */
	{0, 2, -1, 2, 2, 0},

	/* mlock2() with MLOCK_ONFAULT just lock present
	 * pages populated by data.
	 */
	{0, 1, 0, 1, 0, MLOCK_ONFAULT},

	/* fault-in half of pages, lock all with MLOCK_ONFAULT */
	{HPAGES, PAGES, 0, PAGES, HPAGES, MLOCK_ONFAULT},

	/* fault-in 1 page, lock half of pages */
	{1, HPAGES, 1, HPAGES + 1, 1, MLOCK_ONFAULT},

	/* fault-in half, lock little less than 2 pages */
	{HPAGES, HPAGES, -1, HPAGES, HPAGES, MLOCK_ONFAULT},
};

static size_t check_locked_pages(char *addr, size_t len, size_t num_pgs)
{
	size_t n;
	size_t act_pages = 0;

	SAFE_MINCORE(addr, len, vec);

	for (n = 0; n < num_pgs; n++) {
		if (vec[n] & 1)
			act_pages++;
	}

	return act_pages;
}

static void verify_mlock2(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	size_t bsize, asize, act_vmlcks, act_pgs;
	char *addr;

	addr = SAFE_MMAP(NULL, PAGES * pgsz, PROT_WRITE,
			 MAP_SHARED | MAP_ANONYMOUS, 0, 0);

	if (tc->populate_pages)
		memset(addr, 0, tc->populate_pages * pgsz);

	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmLck: %ld", &bsize);

	TEST(tst_syscall(__NR_mlock2, addr, tc->lock_pages * pgsz + tc->offset,
			 tc->flag));
	SAFE_FILE_LINES_SCANF("/proc/self/status", "VmLck: %ld", &asize);

	if (TST_RET != 0) {
		if (tc->flag && TST_ERR == EINVAL)
			tst_res(TCONF, "mlock2() didn't support MLOCK_ONFAULT");
		else
			tst_res(TFAIL | TTERRNO, "mlock2() failed");
		goto end2;
	}

	act_vmlcks = (asize - bsize) * 1024 / pgsz;
	if (tc->exp_vmlcks != act_vmlcks) {
		tst_res(TFAIL, "VmLck showed wrong %ld pages, expected %ld",
			act_vmlcks, tc->exp_vmlcks);
		goto end1;
	}

	act_pgs = check_locked_pages(addr, PAGES * pgsz, PAGES);
	if (act_pgs != tc->exp_present_pages) {
		tst_res(TFAIL, "mlock2(%d) locked %ld pages, expected %ld",
			tc->flag, act_pgs, tc->exp_present_pages);
	} else {
		tst_res(TPASS, "mlock2(%d) succeeded in locking %ld pages",
			tc->flag, tc->exp_present_pages);
	}

end1:
	SAFE_MUNLOCK(addr, tc->lock_pages * pgsz + tc->offset);
end2:
	SAFE_MUNMAP(addr, PAGES * pgsz);
}

static void setup(void)
{
	pgsz = getpagesize();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_mlock2,
	.setup = setup,
	.needs_root = 1,
};
