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
 *	hugeshmget02 - check for ENOENT, EEXIST and EINVAL errors
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
static int shm_nonexistent_key = -1;
static key_t shmkey2;

static long hugepages = 128;
static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static struct tcase {
	int *skey;
	int size_coe;
	int flags;
	int error;
} tcases[] = {
	/* EINVAL - size is 0 */
	{&shmkey2, 0, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW, EINVAL},
	/* EINVAL - size is larger than created segment */
	{&shmkey, 2, SHM_HUGETLB | SHM_RW, EINVAL},
	/* EEXIST - the segment exists and IPC_CREAT | IPC_EXCL is given */
	{&shmkey, 1, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW, EEXIST},
	/* ENOENT - no segment exists for the key and IPC_CREAT is not given */
	/* use shm_nonexistend_key (-1) as the key */
	{&shm_nonexistent_key, 1, SHM_HUGETLB | SHM_RW, ENOENT}
};

static void test_hugeshmget(unsigned int i)
{
	int shm_id_2 = -1;

	if (*tcases[i].skey == -1) {
		shm_id_2 = shmget(*(tcases[i].skey), 0, 0);
		if (shm_id_2 != -1)
			shmctl(shm_id_2, IPC_RMID, NULL);
	}

	TEST(shmget(*(tcases[i].skey), tcases[i].size_coe * shm_size,
					tcases[i].flags));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "shmget succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != tcases[i].error) {
		tst_res(TFAIL | TTERRNO,
			"shmget failed unexpectedly, expected %s",
			tst_strerrno(tcases[i].error));
		return;
	}

	tst_res(TPASS | TTERRNO, "shmget failed as expected");
}

void setup(void)
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
	shmkey2 = shmkey + 1;
	shm_id_1 = shmget(shmkey, shm_size, IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brk(TBROK | TERRNO, "shmget #setup");
}

void cleanup(void)
{
	rm_shm(shm_id_1);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test = test_hugeshmget,
	.tcnt = ARRAY_SIZE(tcases),
};
