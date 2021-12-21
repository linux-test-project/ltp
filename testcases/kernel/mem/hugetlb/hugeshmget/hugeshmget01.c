// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * DESCRIPTION
 *	hugeshmget01 - test that shmget() correctly creates a large
 *			shared memory segment
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 */

#include <limits.h>
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;

static void test_hugeshmget(void)
{
	struct shmid_ds buf;

	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
		tst_res(TFAIL | TERRNO,	"shmctl(IPC_STAT)");
		return;
	}

	if (buf.shm_segsz != shm_size) {
		tst_res(TFAIL, "seqment size is not correct");
		return;
	}

	if (buf.shm_cpid != getpid()) {
		tst_res(TFAIL, "creator pid is not correct");
		return;
	}

	if ((buf.shm_perm.mode & MODE_MASK) != ((SHM_RW) & MODE_MASK)) {
		tst_res(TFAIL, "segment mode is not correct");
		return;
	}

	tst_res(TPASS, "size, pid & mode are correct");
}

static void setup(void)
{
	long hpage_size;

	if (tst_hugepages == 0)
		tst_brk(TCONF, "No enough hugepages for testing.");

	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	shm_size = hpage_size * tst_hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey();

	shm_id_1 = shmget(shmkey, shm_size,
			SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget");
}

static void cleanup(void)
{
	rm_shm(shm_id_1);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmget,
	.request_hugepages = 128,
};
