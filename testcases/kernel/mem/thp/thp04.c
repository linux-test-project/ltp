// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE-2017-1000405
 *
 * Check for the Huge Dirty Cow vulnerability which allows a userspace process
 * to overwrite the huge zero page. Race fixed in:
 *
 *  commit a8f97366452ed491d13cf1e44241bc0b5740b1f0
 *  Author: Kirill A. Shutemov <kirill.shutemov@linux.intel.com>
 *  Date:   Mon Nov 27 06:21:25 2017 +0300
 *
 *   mm, thp: Do not make page table dirty unconditionally in touch_p[mu]d()
 *
 * More details see the following URL
 * https://medium.com/bindecy/huge-dirty-cow-cve-2017-1000405-110eca132de0
 *
 * On old kernel such as 4.9, it has fixed the Dirty Cow bug but a similar check
 * in huge_memory.c was forgotten.  As a result, remote memory writes to ro regions
 * of memory backed by transparent huge pages cause an infinite loop in the kernel.
 * While in this state the process is stil SIGKILLable, but little else works.
 * It is also a regression test about kernel
 * commit 8310d48b125d("huge_memory.c: respect FOLL_FORCE/FOLL_COW for thp").
 */

#include "tst_test.h"
#include "lapi/mmap.h"
#include "tst_fuzzy_sync.h"

static char *write_thp, *read_thp;
static int *write_ptr, *read_ptr;
static size_t thp_size;
static int writefd = -1, readfd = -1;
static struct tst_fzsync_pair fzsync_pair;

static void *alloc_zero_page(void *baseaddr)
{
	int i;
	void *ret;

	/* Find aligned chunk of address space. MAP_HUGETLB doesn't work. */
	for (i = 0; i < 16; i++, baseaddr += thp_size) {
		ret = mmap(baseaddr, thp_size, PROT_READ,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (ret == baseaddr) {
			TEST(madvise(ret, thp_size, MADV_HUGEPAGE));

			if (TST_RET == -1 && TST_ERR == EINVAL) {
				tst_brk(TCONF | TTERRNO,
					"madvise(MADV_HUGEPAGE) not supported");
			}

			if (TST_RET) {
				tst_brk(TBROK | TTERRNO,
					"madvise(MADV_HUGEPAGE) failed");
			}

			return ret;
		}

		if (ret != MAP_FAILED)
			SAFE_MUNMAP(ret, thp_size);
	}

	tst_brk(TBROK, "Cannot map huge zero page near the specified address");
	return NULL;	/* Silence compiler warning */
}

static void setup(void)
{
	size_t i;

	thp_size = tst_get_hugepage_size();

	if (!thp_size)
		tst_brk(TCONF, "Kernel does not support huge pages");

	write_thp = alloc_zero_page((void *)thp_size);

	for (i = 0; i < thp_size; i++) {
		if (write_thp[i])
			tst_brk(TCONF, "Huge zero page is pre-polluted");
	}

	/* leave a hole between read and write THP to prevent merge */
	read_thp = alloc_zero_page(write_thp + 2 * thp_size);
	write_ptr = (int *)(write_thp + thp_size - sizeof(int));
	read_ptr = (int *)(read_thp + thp_size - sizeof(int));
	writefd = SAFE_OPEN("/proc/self/mem", O_RDWR);
	readfd = SAFE_OPEN("/proc/self/mem", O_RDWR);

	fzsync_pair.exec_loops = 100000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	int c;

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		madvise(write_thp, thp_size, MADV_DONTNEED);
		memcpy(&c, write_ptr, sizeof(c));
		SAFE_LSEEK(readfd, (off_t)write_ptr, SEEK_SET);
		SAFE_READ(1, readfd, &c, sizeof(int));
		tst_fzsync_end_race_b(&fzsync_pair);
		/* Wait for dirty page handling before next madvise() */
		usleep(10);
	}

	return arg;
}

static void run(void)
{
	int c = 0xdeadbeef;

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		/* Write into the main huge page */
		tst_fzsync_start_race_a(&fzsync_pair);
		SAFE_LSEEK(writefd, (off_t)write_ptr, SEEK_SET);
		madvise(write_thp, thp_size, MADV_DONTNEED);
		SAFE_WRITE(SAFE_WRITE_ALL, writefd, &c, sizeof(int));
		tst_fzsync_end_race_a(&fzsync_pair);

		/* Check the other huge zero page for pollution */
		madvise(read_thp, thp_size, MADV_DONTNEED);

		if (*read_ptr != 0) {
			tst_res(TFAIL, "Huge zero page was polluted");
			return;
		}
	}

	tst_res(TPASS, "Huge zero page is still clean");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);

	if (readfd >= 0)
		SAFE_CLOSE(readfd);

	if (writefd >= 0)
		SAFE_CLOSE(writefd);

	if (read_thp)
		SAFE_MUNMAP(read_thp, thp_size);
	if (write_thp)
		SAFE_MUNMAP(write_thp, thp_size);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.max_runtime = 150,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a8f97366452e"},
		{"linux-git", "8310d48b125d"},
		{"CVE", "2017-1000405"},
		{}
	}
};
