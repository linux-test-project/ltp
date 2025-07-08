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
 * longer accessible.
 *
 * munmap call should succeed to unmap a mapped file or anonymous shared
 * memory region from the process's address space and it returns with a
 * value 0, further reference to the unmapped region should result in a
 * segmentation fault (SIGSEGV).
 */

#include "tst_test.h"

static int fd = -1;
static char *map_addr;
static unsigned int map_len;

static void run(void)
{
	int status;

	SAFE_MUNMAP(map_addr, map_len);

	/*
	 * Check whether further reference is possible to the unmapped memory
	 * region by writing to the first byte of region with some arbitrary
	 * number.
	 */
	if (!SAFE_FORK()) {
		*map_addr = 50;
		_exit(0);
	}

	map_addr = NULL;

	SAFE_WAIT(&status);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Child was unable to access unmapped memory");
		return;
	}
	tst_res(TFAIL, "Child succeeds to refer unmapped memory region");
}

static void setup(void)
{
	size_t page_sz = SAFE_SYSCONF(_SC_PAGESIZE);

	map_len = 3 * page_sz;

	fd = SAFE_OPEN("mmapfile", O_RDWR | O_CREAT, 0666);

	SAFE_LSEEK(fd, map_len, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "a", 1);

	map_addr = SAFE_MMAP(0, map_len, PROT_READ | PROT_WRITE,
			     MAP_FILE | MAP_SHARED, fd, 0);
}

static void cleanup(void)
{
	if (map_addr)
		SAFE_MUNMAP(map_addr, map_len);
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
