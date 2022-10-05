// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Red Hat, Inc.
 *
 */

/*
 * Test description: Verify msync() after writing into mmap()-ed file works.
 *
 * Write to mapped region and sync the memory back with file. Check the page
 * is no longer dirty after msync() call.
 */

#include <errno.h>
#include "tst_test.h"

static int test_fd;
static char *mmaped_area;
static long pagesize;

#define STRING_TO_WRITE	"AAAAAAAAAA"

uint64_t get_dirty_bit(void *data)
{
	int pagemap_fd, pageflags_fd;
	unsigned long addr;
	uint64_t pagemap_entry, pageflag_entry, pfn, index;

	addr = (unsigned long)data;
	index = (addr / pagesize) * sizeof(uint64_t);
	pagemap_fd = SAFE_OPEN("/proc/self/pagemap", O_RDONLY);
	SAFE_LSEEK(pagemap_fd, index, SEEK_SET);
	SAFE_READ(1, pagemap_fd, &pagemap_entry, sizeof(pagemap_entry));
	SAFE_CLOSE(pagemap_fd);
	pfn = pagemap_entry & ((1ULL << 55) - 1);
	if (!pfn)
		return 0;
	pageflags_fd = SAFE_OPEN("/proc/kpageflags", O_RDONLY);
	index = pfn * sizeof(uint64_t);
	SAFE_LSEEK(pageflags_fd, index, SEEK_SET);
	SAFE_READ(1, pageflags_fd, &pageflag_entry, sizeof(pageflag_entry));
	SAFE_CLOSE(pageflags_fd);
	return pageflag_entry & (1ULL << 4);
}

static void test_msync(void)
{
	uint64_t dirty;

	test_fd = SAFE_OPEN("msync04/testfile", O_CREAT | O_TRUNC | O_RDWR,
		0644);
	SAFE_WRITE(SAFE_WRITE_ANY, test_fd, STRING_TO_WRITE, sizeof(STRING_TO_WRITE) - 1);
	mmaped_area = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE,
			MAP_SHARED, test_fd, 0);
	SAFE_CLOSE(test_fd);
	mmaped_area[8] = 'B';
	dirty = get_dirty_bit(mmaped_area);
	if (!dirty) {
		tst_res(TFAIL, "Expected dirty bit to be set after writing to"
				" mmap()-ed area");
		goto clean;
	}
	if (msync(mmaped_area, pagesize, MS_SYNC) < 0) {
		tst_res(TFAIL | TERRNO, "msync() failed");
		goto clean;
	}
	dirty = get_dirty_bit(mmaped_area);
	if (dirty)
		tst_res(TFAIL, "msync() failed to write dirty page despite"
				" succeeding");
	else
		tst_res(TPASS, "msync() working correctly");

clean:
	SAFE_MUNMAP(mmaped_area, pagesize);
	mmaped_area = NULL;
}

static void setup(void)
{
	pagesize = (off_t)SAFE_SYSCONF(_SC_PAGESIZE);
}

static void cleanup(void)
{
	if (mmaped_area)
		SAFE_MUNMAP(mmaped_area, pagesize);

	if (test_fd > 0)
		SAFE_CLOSE(test_fd);
}

static struct tst_test test = {
	.test_all = test_msync,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mntpoint = "msync04",
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *[]) {
		"tmpfs",
		NULL
	},
	.min_kver = "2.6.25",
};
