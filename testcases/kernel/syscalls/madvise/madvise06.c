/*
 * Copyright (c) 2016 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DESCRIPTION
 *
 *   Page fault occurs in spite that madvise(WILLNEED) system call is called
 *   to prefetch the page. This issue is reproduced by running a program
 *   which sequentially accesses to a shared memory and calls madvise(WILLNEED)
 *   to the next page on a page fault.
 *
 *   This bug is present in all RHEL7 versions. It looks like this was fixed in
 *   mainline kernel > v3.15 by the following patch:
 *
 *   commit 55231e5c898c5c03c14194001e349f40f59bd300
 *   Author: Johannes Weiner <hannes@cmpxchg.org>
 *   Date:   Thu May 22 11:54:17 2014 -0700
 *
 *       mm: madvise: fix MADV_WILLNEED on shmem swapouts
 */

#include <errno.h>
#include <sys/sysinfo.h>
#include "tst_test.h"

#define GB_SZ  (1024*1024*1024)

static long dst_max;
static int pg_sz;

static void setup(void)
{
	struct sysinfo sys_buf;

	sysinfo(&sys_buf);

	if (sys_buf.totalram < 2L * GB_SZ)
		tst_brk(TCONF, "Test requires more than 2GB of RAM");
	if (sys_buf.totalram > 100L * GB_SZ)
		tst_brk(TCONF, "System RAM is too large, skip test");

	dst_max = sys_buf.totalram / GB_SZ;
	tst_res(TINFO, "dst_max = %ld", dst_max);

	pg_sz = getpagesize();
}

static int get_page_fault_num(void)
{
	int pg;

	SAFE_FILE_SCANF("/proc/self/stat",
			"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d",
			&pg);

	return pg;
}

static void test_advice_willneed(void)
{
	int i;
	char *src;
	char *dst[100];
	int page_fault_num_1;
	int page_fault_num_2;

	/* allocate source memory (1GB only) */
	src = SAFE_MMAP(NULL, 1 * GB_SZ, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS,
			-1, 0);

	/* allocate destination memory (array) */
	for (i = 0; i < dst_max; ++i)
		dst[i] = SAFE_MMAP(NULL, 1 * GB_SZ,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS,
				-1, 0);

	/* memmove source to each destination memories (for SWAP-OUT) */
	for (i = 0; i < dst_max; ++i)
		memmove(dst[i], src, 1 * GB_SZ);

	tst_res(TINFO, "PageFault(no madvice): %d", get_page_fault_num());

	/* Do madvice() to dst[0] */
	TEST(madvise(dst[0], pg_sz, MADV_WILLNEED));
	if (TEST_RETURN == -1)
		tst_brk(TBROK | TERRNO, "madvise failed");

	page_fault_num_1 = get_page_fault_num();
	tst_res(TINFO, "PageFault(madvice / no mem access): %d",
			page_fault_num_1);

	*dst[0] = 'a';
	page_fault_num_2 = get_page_fault_num();
	tst_res(TINFO, "PageFault(madvice / mem access): %d",
			page_fault_num_2);

	if (page_fault_num_1 != page_fault_num_2)
		tst_res(TFAIL, "Bug has been reproduced");
	else
		tst_res(TPASS, "Regression test pass");

	SAFE_MUNMAP(src, 1 * GB_SZ);
	for (i = 0; i < dst_max; ++i)
		SAFE_MUNMAP(dst[i], 1 * GB_SZ);
}

static struct tst_test test = {
	.tid = "madvice06",
	.test_all = test_advice_willneed,
	.setup = setup,
};
