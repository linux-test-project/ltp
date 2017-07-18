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
 *	hugeshmat02 - check for EINVAL and EACCES errors with hugetlb
 *
 * ALGORITHM
 *	loop if that option was specified
 *	  call shmat() using three invalid test cases
 *	  check the errno value
 *	    issue a PASS message if we get EINVAL or EACCES
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated By Robbie Williamson
 *
 * RESTRICTIONS
 *	Must be ran as root
 */

#include <pwd.h>
#include <limits.h>
#include "hugetlb.h"
#include "mem.h"
#include "hugetlb.h"

#if __WORDSIZE == 64
#define NADDR	0x10000000eef	/* a 64bit non alligned address value */
#else
#define NADDR	0x60000eef	/* a non alligned address value */
#endif

static size_t shm_size;
static int shm_id_1 = -1;
static int shm_id_2 = -1;
static void *addr;

static long hugepages = 128;

static struct tst_option options[] = {
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

struct tcase {
	int *shmid;
	void *addr;
	int error;
} tcases[] = {
	/* EINVAL - the shared memory ID is not valid */
	{&shm_id_1, NULL, EINVAL},
	/* EINVAL - the address is not page aligned and SHM_RND is not given */
	{&shm_id_2, (void *)NADDR, EINVAL}
};

static void verify_hugeshmat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	addr = shmat(*(tc->shmid), tc->addr, 0);
	if (addr != (void *)-1) {
		tst_res(TFAIL, "shmat suceeded unexpectedly");
		return;
	}

	if (errno == tc->error) {
		tst_res(TPASS | TERRNO, "shmat failed as "
				"expected");
	} else {
		tst_res(TFAIL | TERRNO, "shmat failed "
				"unexpectedly - expect errno=%d, "
				"got", tc->error);
	}
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

	/* create a shared memory resource with read and write permissions */
	/* also post increment the shmkey for the next shmget call */
	shm_id_2 = shmget(shmkey++, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_2 == -1)
		tst_brk(TBROK | TERRNO, "shmget");
}

void cleanup(void)
{
	rm_shm(shm_id_2);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = options,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_hugeshmat,
	.setup = setup,
	.cleanup = cleanup,
};
