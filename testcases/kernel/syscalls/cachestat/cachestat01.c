// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that cachestat() syscall is properly counting cached pages
 * written inside a file. If storage device synchronization is requested, test
 * will check if the number of dirty pages is zero.
 *
 * [Algorithm]
 *
 * - create a file with specific amount of pages
 * - synchronize storage device, if needed
 * - monitor file with cachestat()
 * - check if the right amount of pages have been moved into cache
 * - if storage device synchronization is requested, check that dirty pages is
 *    zero
 */

#include <stdlib.h>
#include "cachestat.h"

#define MNTPOINT "mntpoint"
#define FILENAME MNTPOINT "/myfile.bin"

static int page_size, num_shift;
static char *page_data;
static struct cachestat *cs;
static struct cachestat_range *cs_range;

static void test_cached_pages(const unsigned int use_sync, const int num_pages)
{
	int fd;

	tst_res(TINFO, "%s file synchronization", use_sync ? "Enable" : "Disable");
	tst_res(TINFO, "Number of pages: %d", num_pages);

	memset(cs, 0, sizeof(struct cachestat));

	fd = SAFE_OPEN(FILENAME, O_RDWR | O_CREAT, 0600);

	for (int i = 0; i < num_pages; i++)
		SAFE_WRITE(0, fd, page_data, page_size);

	if (use_sync)
		fsync(fd);

	cs_range->off = 0;
	cs_range->len = page_size * num_pages;

	memset(cs, 0xff, sizeof(*cs));

	TST_EXP_PASS(cachestat(fd, cs_range, cs, 0));
	print_cachestat(cs);

	TST_EXP_EQ_LI(cs->nr_cache + cs->nr_evicted, num_pages);

	if (use_sync)
		TST_EXP_EQ_LI(cs->nr_dirty, 0);

	SAFE_CLOSE(fd);
	SAFE_UNLINK(FILENAME);
}

static void run(unsigned int use_sync)
{
	for (int i = 0; i < num_shift; i++)
		test_cached_pages(use_sync, 1 << i);
}

static void setup(void)
{
	page_size = (int)sysconf(_SC_PAGESIZE);
	num_shift = MIN(tst_device->size*1024*2.5/page_size, 15);
	page_data = SAFE_MALLOC(page_size);
	memset(page_data, 'a', page_size);
}

static void cleanup(void)
{
	free(page_data);
}

static struct tst_test test = {
	.timeout = 13,
	.test = run,
	.tcnt = 2,
	.setup = setup,
	.cleanup = cleanup,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []) {
		"fuse",
		"tmpfs",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&cs, .size = sizeof(struct cachestat)},
		{&cs_range, .size = sizeof(struct cachestat_range)},
		{}
	},
};
