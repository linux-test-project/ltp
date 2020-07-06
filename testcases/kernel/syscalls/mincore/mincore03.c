// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2020
 * Email: code@zilogic.com
 */

/*
 * mincore03
 * Testcase 1: Test shows that pages mapped as anonymous and
 * not faulted, are reported as not resident in memory by mincore().
 * Testcase 2: Test shows that pages mapped as anonymous and faulted,
 * are reported as resident in memory by mincore().
 */

#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include "tst_test.h"

#define NUM_PAGES 3

static struct tcase {
	bool mlock;
	int expected_pages;
	char *desc;
} tcases[] = {
	{ false, 0, "untouched pages are not resident"},
	{ true, NUM_PAGES, "locked pages are resident"},
};

static int size, page_size;
static void *ptr;

static void cleanup(void)
{
	if (ptr)
		SAFE_MUNMAP(ptr, size);
}

static void setup(void)
{
	page_size = getpagesize();
	size = page_size * NUM_PAGES;
}

static void test_mincore(unsigned int test_nr)
{
	const struct tcase *tc = &tcases[test_nr];
	unsigned char vec[NUM_PAGES];
	int locked_pages;
	int count, mincore_ret;

	ptr = SAFE_MMAP(NULL, size,  PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if (tc->mlock)
		SAFE_MLOCK(ptr, size);

	mincore_ret = mincore(ptr, size, vec);
	if (mincore_ret == -1)
		tst_brk(TBROK | TERRNO, "mincore failed");
	locked_pages = 0;
	for (count = 0; count < NUM_PAGES; count++)
		if (vec[count] & 1)
			locked_pages++;

	if (locked_pages == tc->expected_pages)
		tst_res(TPASS, "mincore() reports %s", tc->desc);
	else
		tst_res(TFAIL, "mincore reports resident pages as %d, but expected %d",
			locked_pages, tc->expected_pages);

	if (tc->mlock)
		SAFE_MUNLOCK(ptr, size);
	SAFE_MUNMAP(ptr, size);
	ptr = NULL;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = test_mincore,
};

