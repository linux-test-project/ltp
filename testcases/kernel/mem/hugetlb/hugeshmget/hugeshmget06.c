// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * This testcase creates shared memory segments backed by hugepages,
 * writes specific patterns to each segment, verifies pattern,
 * and detaches a shared memory segments in a loop.
 * It ensures that the hugepage backed shared memory functionalities
 * works correctly by validating the data written to segment.
 */

#include "hugetlb.h"
#include "tst_safe_sysv_ipc.h"

#define NR_HUGEPAGES 4

static long hpage_size;
static int shmid = -1;

static void run_test(void)
{
	size_t i, j;
	char pattern;
	char *shmaddr;

	shmaddr = SAFE_SHMAT(shmid, 0, SHM_RND);
	tst_res(TINFO, "shmaddr: %p", shmaddr);

	for (i = 0; i < NR_HUGEPAGES; i++) {
		pattern = 65 + (i % 26);
		tst_res(TDEBUG, "Touching %p with %c",
		shmaddr + (i * hpage_size), pattern);
		memset(shmaddr + (i * hpage_size), pattern, hpage_size);
	}

	for (i = 0; i < NR_HUGEPAGES; i++) {
		pattern = 65 + (i % 26);
		tst_res(TDEBUG, "Verifying %p", (shmaddr + (i * hpage_size)));
		for (j = 0; j < (size_t)hpage_size; j++)
			if (*(shmaddr + (i * hpage_size) + j) != pattern) {
				tst_res(TFAIL, "Got wrong byte 0x%02x expected 0x%02x",
						*(shmaddr + (i * hpage_size) + j),
						pattern);
				return;
			}
	}
	SAFE_SHMDT((const void *)shmaddr);
	tst_res(TPASS, "shm hugepages works correctly");
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
	tst_res(TINFO, "hugepage size is %ld", hpage_size);
	shmid = SAFE_SHMGET(IPC_PRIVATE, NR_HUGEPAGES * hpage_size, SHM_HUGETLB|IPC_CREAT|SHM_R|SHM_W);
	tst_res(TINFO, "shmid: 0x%x", shmid);
}

static void cleanup(void)
{
	if (shmid >= 0)
		SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {NR_HUGEPAGES, TST_NEEDS},
};
