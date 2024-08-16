// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Xiao Yang <ice_yangxiao@163.com>
 */

/*\
 * [Description]
 *
 * Check various errnos for mseal(2).
 *
 * - mseal() fails with EINVAL if flags is invalid.
 * - mseal() fails with EINVAL if the start address is not page aligned.
 * - mseal() fails with EINVAL if address range overflows.
 * - mseal() fails with ENOMEM if the start address is not allocated.
 * - mseal() fails with ENOMEM if the end address is not allocated.
 * - mseal() fails with ENOMEM if there is a gap (unallocated memory) between start and end address.
 *
 * TODO: support both raw syscall and libc wrapper via .test_variants.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/syscalls.h"

static void *start_addr, *unaligned_start_addr, *unallocated_start_addr, *unallocated_end_addr;
static size_t page_size, twopages_size, fourpages_size, overflow_size;

static struct tcase {
	void **addr;
	size_t *len;
	unsigned long flags;
	int exp_err;
} tcases[] = {
	{&start_addr, &page_size, ULONG_MAX, EINVAL},
	{&unaligned_start_addr, &page_size, 0, EINVAL},
	{&start_addr, &overflow_size, 0, EINVAL},
	{&unallocated_start_addr, &twopages_size, 0, ENOMEM},
	{&unallocated_end_addr, &twopages_size, 0, ENOMEM},
	{&start_addr, &fourpages_size, 0, ENOMEM},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(tst_syscall(__NR_mseal, *tc->addr, *tc->len, tc->flags), tc->exp_err,
		"mseal(%p, %lu, %lu)", *tc->addr, *tc->len, tc->flags);
}

static void setup(void)
{
	page_size = getpagesize();
	twopages_size = page_size * 2;
	fourpages_size = page_size * 4;
	overflow_size = ULONG_MAX - page_size + 2;
	start_addr = SAFE_MMAP(NULL, fourpages_size, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	unaligned_start_addr = start_addr + 1;
	SAFE_MUNMAP(start_addr + twopages_size, page_size);
	unallocated_start_addr = start_addr + twopages_size;
	unallocated_end_addr = start_addr + page_size;
}

static void cleanup(void)
{
	SAFE_MUNMAP(start_addr, fourpages_size);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
};
