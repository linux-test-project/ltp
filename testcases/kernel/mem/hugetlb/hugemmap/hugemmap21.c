// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * Tests copy-on-write semantics of large pages where a number of threads
 * map the same file with the MAP_PRIVATE flag. The threads then write
 * into their copy of the mapping and recheck the contents to ensure they
 * were not corrupted by the other threads.
 */

#include "hugetlb.h"

#define THREADS 5
#define NR_HUGEPAGES 6
#define MNTPOINT "hugetlbfs/"

static int fd = -1;
static long hpage_size;

static void do_work(int thread, size_t size, int fd)
{
	char *addr;
	size_t i;
	char pattern = thread+65;

	addr = SAFE_MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

	tst_res(TINFO, "Thread %d (pid %d): Mapped at address %p",
	       thread, getpid(), addr);

	for (i = 0; i < size; i++)
		memcpy((char *)addr+i, &pattern, 1);

	SAFE_MSYNC(addr, size, MS_SYNC);

	for (i = 0; i < size; i++) {
		if (addr[i] != pattern) {
			tst_res(TFAIL, "thread %d (pid: %d): Corruption at %p; "
				   "Got %c, Expected %c", thread, getpid(),
				   &addr[i], addr[i], pattern);
			goto cleanup;
		}
	}

	tst_res(TINFO, "Thread %d (pid %d): Pattern verified",
			thread, getpid());

cleanup:
	SAFE_MUNMAP(addr, size);
	exit(0);
}

static void run_test(void)
{
	int i, pid;
	char *addr;
	size_t size, itr;
	pid_t *wait_list;

	wait_list = SAFE_MALLOC(THREADS * sizeof(pid_t));
	size = (NR_HUGEPAGES / (THREADS+1)) * hpage_size;

	/*
	 * First, mmap the file with MAP_SHARED and fill with data
	 * If this is not done, then the fault handler will not be
	 * called in the kernel since private mappings will be
	 * created for the children at prefault time.
	 */
	addr = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	for (itr = 0; itr < size; itr += 8)
		memcpy(addr+itr, "deadbeef", 8);

	for (i = 0; i < THREADS; i++) {
		pid = SAFE_FORK();

		if (pid == 0)
			do_work(i, size, fd);

		wait_list[i] = pid;
	}
	tst_reap_children();

	SAFE_MUNMAP(addr, size);
	free(wait_list);
	tst_res(TPASS, "mmap COW working as expected.");
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd >= 1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {NR_HUGEPAGES, TST_NEEDS},
};
