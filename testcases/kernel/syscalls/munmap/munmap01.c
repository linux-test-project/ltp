// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that, munmap call will succeed to unmap a mapped file or
 * anonymous shared memory region from the calling process's address space
 * and after successful completion of munmap, the unmapped region is no
 * longer accessible (even if partially unmapped).
 *
 * munmap call should succeed to unmap a part or all of the mapped region of
 * a file or anonymous shared memory region from the process's address space
 * and it returns with a value 0, further reference to the unmapped region
 * should result in a segmentation fault (SIGSEGV).
 */

#include "tst_test.h"

static size_t page_sz;
static int fd = -1;
static char *map_base;
static char *map_addr;
static unsigned int map_len;
static unsigned int map_len_full;
static const char *const variants[] = {
	"unmapped region not accessible",
	"partially unmapped region not accessible",
};

static void run(void)
{
	int status;

	SAFE_MUNMAP(map_addr, map_len);
	map_base = NULL;

	/*
	 * Check whether further reference is possible to the unmapped memory
	 * region by writing to the first byte of region with some arbitrary
	 * number.
	 */
	if (!SAFE_FORK()) {
		*map_addr = 50;
		_exit(0);
	}

	SAFE_WAIT(&status);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Child was unable to access unmapped memory");
		return;
	}
	tst_res(TFAIL, "Child succeeds to refer unmapped memory region");
}

static void setup(void)
{
	page_sz = SAFE_SYSCONF(_SC_PAGESIZE);

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant]);

	map_len_full = 3 * page_sz;
	map_len = map_len_full;

	fd = SAFE_OPEN("mmapfile", O_RDWR | O_CREAT, 0666);

	SAFE_LSEEK(fd, map_len, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "a", 1);

	map_base = SAFE_MMAP(0, map_len, PROT_READ | PROT_WRITE,
			     MAP_FILE | MAP_SHARED, fd, 0);
	map_addr = map_base;

	if (tst_variant) {
		map_addr = map_addr + page_sz;
		map_len = map_len - page_sz;
	}
}

static void cleanup(void)
{
	if (map_base) {
		SAFE_MUNMAP(map_base, map_len_full);
	} else {
		if (tst_variant)
			SAFE_MUNMAP(map_addr, page_sz);
	}

	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.test_variants = ARRAY_SIZE(variants),
};
