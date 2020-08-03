// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2020
 * Email: code@zilogic.com
 */

/*
 * mincore04
 * Test shows that pages mapped in one process(parent) and
 * faulted in another(child) results in mincore(in parent) reporting
 * that all mapped pages are resident.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include "tst_test.h"

#define NUM_PAGES 3

static int fd;
static int size;
static void *ptr;

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
	if (ptr) {
		SAFE_MUNLOCK(ptr, size);
		SAFE_MUNMAP(ptr, size);
	}
}

static void setup(void)
{
	int page_size, ret;

	page_size = getpagesize();
	size = page_size * NUM_PAGES;
	fd = SAFE_OPEN("FILE", O_CREAT | O_RDWR, 0600);
	SAFE_FTRUNCATE(fd, size);

	/* File pages from file creation are cleared from cache. */
	SAFE_FSYNC(fd);
	ret = posix_fadvise(fd, 0, size, POSIX_FADV_DONTNEED);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "fadvise failed");
}

static void lock_file(void)
{
	SAFE_MLOCK(ptr, size);
	TST_CHECKPOINT_WAKE(0);
	TST_CHECKPOINT_WAIT(1);
}

static int count_pages_in_cache(void)
{
	int locked_pages = 0;
	int count, ret;
	unsigned char vec[NUM_PAGES];

	TST_CHECKPOINT_WAIT(0);

	ret = mincore(ptr, size, vec);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "mincore failed");
	for (count = 0; count < NUM_PAGES; count++) {
		if (vec[count] & 1)
			locked_pages++;
	}

	TST_CHECKPOINT_WAKE(1);
	return locked_pages;
}

static void test_mincore(void)
{
	int  locked_pages;

	ptr = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	pid_t child_pid = SAFE_FORK();

	if (child_pid == 0) {
		lock_file();
		exit(0);
	}

	locked_pages = count_pages_in_cache();
	tst_reap_children();

	if (locked_pages == NUM_PAGES)
		tst_res(TPASS, "mincore reports all %d pages locked by child process "
			"are resident", locked_pages);
	else
		tst_res(TFAIL, "mincore reports %d pages resident but %d pages "
			"locked by child process", locked_pages, NUM_PAGES);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test_all = test_mincore,
	.needs_checkpoints = 1,
};
