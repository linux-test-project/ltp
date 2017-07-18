/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

/*
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
#include "mem.h"
#include "hugetlb.h"

static size_t shm_size;
static int shm_id_1 = -1;

static long hugepages = 128;
static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

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

	check_hugepage();
	if (nr_opt)
		hugepages = SAFE_STRTOL(nr_opt, 0, LONG_MAX);

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
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
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugeshmget,
};
