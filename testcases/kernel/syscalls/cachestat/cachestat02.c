// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that cachestat() syscall is properly counting cached pages
 * written inside a shared memory.
 *
 * [Algorithm]
 *
 * - create a shared memory with a specific amount of pages
 * - monitor file with cachestat()
 * - check if the right amount of pages have been moved into cache
 */

#include <stdlib.h>
#include "cachestat.h"

#define FILENAME "myfile.bin"

static int page_size;
static char *page_data;
static struct cachestat *cs;
static struct cachestat_range *cs_range;

static void test_cached_pages(const int num_pages)
{
	int fd, file_size;

	tst_res(TINFO, "Number of pages: %d", num_pages);

	memset(cs, 0, sizeof(struct cachestat));

	fd = shm_open(FILENAME, O_RDWR | O_CREAT, 0600);
	if (fd < 0)
		tst_brk(TBROK | TERRNO, "shm_open error");

	file_size = page_size * num_pages;

	cs_range->off = 0;
	cs_range->len = file_size;

	SAFE_FTRUNCATE(fd, file_size);
	for (int i = 0; i < num_pages; i++)
		SAFE_WRITE(0, fd, page_data, page_size);

	memset(cs, 0xff, sizeof(*cs));

	TST_EXP_PASS(cachestat(fd, cs_range, cs, 0));
	print_cachestat(cs);

	TST_EXP_EQ_LI(cs->nr_cache + cs->nr_evicted, num_pages);

	SAFE_CLOSE(fd);
	shm_unlink(FILENAME);
}

static void run(void)
{
	for (int i = 0; i < 10; i++)
		test_cached_pages(1 << i);
}

static void setup(void)
{
	page_size = (int)sysconf(_SC_PAGESIZE);

	page_data = SAFE_MALLOC(page_size);
	memset(page_data, 'a', page_size);
}

static void cleanup(void)
{
	free(page_data);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&cs, .size = sizeof(struct cachestat)},
		{&cs_range, .size = sizeof(struct cachestat_range)},
		{}
	},
};
