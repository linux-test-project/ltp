// SPDX-License-Identifier: GPL-2.0-or-late
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * DESCRIPTION
 *	hugeshmget03 - test for ENOSPC error
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 */

#include <limits.h>
#include "hugetlb.h"

/*
 * The MAXIDS value is somewhat arbitrary and may need to be increased
 * depending on the system being tested.
 */
#define MAXIDS	8192
#define PATH_SHMMNI	"/proc/sys/kernel/shmmni"

static size_t shm_size;
static int shm_id_1 = -1;
static int num_shms;
static int shm_id_arr[MAXIDS];

static long orig_shmmni = -1;
static struct tst_option options[] = {
	{"s:", &nr_opt, "-s num   Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static void test_hugeshmget(void)
{
	TEST(shmget(IPC_PRIVATE, shm_size,
				SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW));
	if (TST_RET != -1) {
		tst_res(TFAIL, "shmget succeeded unexpectedly");
		return;
	}
	if (TST_ERR == ENOSPC)
		tst_res(TPASS | TTERRNO, "shmget failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "shmget failed unexpectedly "
				"- expect errno=ENOSPC, got");
}

static void setup(void)
{
	long hpage_size;

	if (tst_hugepages == 0)
		tst_brk(TCONF, "No enough hugepages for testing.");

	SAFE_FILE_SCANF(PATH_SHMMNI, "%ld", &orig_shmmni);
	SAFE_FILE_PRINTF(PATH_SHMMNI, "%ld", tst_hugepages / 2);

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	shm_size = hpage_size;

	/*
	 * Use a while loop to create the maximum number of memory segments.
	 * If the loop exceeds MAXIDS, then break the test and cleanup.
	 */
	num_shms = 0;
	shm_id_1 = shmget(IPC_PRIVATE, shm_size,
			  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	while (shm_id_1 != -1) {
		shm_id_arr[num_shms++] = shm_id_1;
		if (num_shms == MAXIDS)
			tst_brk(TBROK, "The maximum number of "
				 "shared memory ID's has been reached. "
				 "Please increase the MAXIDS value in "
				 "the test.");
		shm_id_1 = shmget(IPC_PRIVATE, shm_size,
				  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	}
	if (errno != ENOSPC)
		tst_brk(TBROK | TERRNO, "shmget #setup");
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < num_shms; i++)
		rm_shm(shm_id_arr[i]);

	if (orig_shmmni != -1)
		FILE_PRINTF(PATH_SHMMNI, "%ld", orig_shmmni);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmget,
	.request_hugepages = 128,
};
